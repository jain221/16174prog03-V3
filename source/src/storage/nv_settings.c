/**
 * @file  nv_settings.c
 * @brief Non-Volatile Settings -- stored in EEPROM
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "nv_settings.h"

#include "alc_logger.h"
#include "alc_number_utils.h"
#include "alc_store_string_utils.h"
#include "eeprom_arch.h"
#include "lib/crc16.h"
#include "modem_ctrl.h"




/*******************************************************************************
*                               LOCAL DEFINES
*******************************************************************************/

/* Location of data in the EEPROM */
#define EEPROM_CLOUD_IPV4_ADDR              0U
#define EEPROM_CLOUD_PORTNUM_ADDR           4U
#define EEPROM_MODEM_APN_ADDR               32U
#define EEPROM_MODEM_USERNAME_ADDR          64U
#define EEPROM_MODEM_PASSWORD_ADDR          96U
#define EEPROM_RADIO_PAN_CH_ADDR            128U    /* size 4 bytes */
#define EEPROM_RADIO_PAN_ID_ADDR            132U    /* size 4 bytes */


/* Size of the data in the EEPROM */
#define EEPROM_MODEM_APN_MAXLEN             31U
#define EEPROM_MODEM_USERNAME_MAXLEN        31U
#define EEPROM_MODEM_PASSWORD_MAXLEN        31U




/*******************************************************************************
*                               LOCAL CONSTANTS
*******************************************************************************/




/*******************************************************************************
*                               LOCAL DATA TYPES
*******************************************************************************/

typedef struct __attribute__((packed)) {
    struct __attribute__((packed)) {
        uint16_t value;
    } data;
    uint16_t crc16;
} AlcStoreU16;




/*******************************************************************************
*                               LOCAL TABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL GLOBAL VARIABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static bool read_u16_from_eeprom__(eeprom_addr_t address, uint16_t *p_value);
static bool write_u16_to_eeprom__(eeprom_addr_t address, uint16_t value);




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
/** @brief Read the IPv4 address of the cloud server from EEPROM memory
 */
bool Store_read_cloud_ipv4(uint8_t *p_buff4)
{
    bool success=false;

    if(p_buff4)
    {
        eeprom_read(EEPROM_CLOUD_IPV4_ADDR, p_buff4, 4);

        success = eeprom_last_op_success();

        if(!success)
        {
            /* Failed to read data */
            memset(p_buff4, 0, 4);
        }
    }

    return success;
}
/******************************************************************************/
/** @brief Write the IPv4 address of the cloud server to EEPROM memory
 */
bool Store_write_cloud_ipv4(uint8_t const *p_buff4)
{
    bool success=false;

    if(p_buff4)
    {
        if(eeprom_enable_write())
        {
            eeprom_write(EEPROM_CLOUD_IPV4_ADDR, p_buff4, 4);

            success = eeprom_last_op_success();
        }

        eeprom_disable_write();
    }

    if(success)
    {
        AlcLogger_log_info("Cloud IP address updated in EEPROM");
        ModemCtrl_conf_has_been_changed();
    }
    else
    {
        AlcLogger_log_error("Failed to store Cloud IP address in EEPROM");
    }

    return success;
}
/******************************************************************************/
/** @brief Read the port number of the cloud server from EEPROM memory
 */
bool Store_read_cloud_portnum(uint16_t *p_portnum)
{
    bool success=false;

    if(p_portnum)
    {
        uint8_t buff[2];

        eeprom_read(EEPROM_CLOUD_PORTNUM_ADDR, buff, sizeof(buff));

        success = eeprom_last_op_success();

        if(success)
        {
            *p_portnum = ALC_MAKE16(buff[0], buff[1]);
        }
        else
        {
            *p_portnum = 0U;
        }
    }

    return success;
}
/******************************************************************************/
/** @brief Write the port number of the cloud server to EEPROM memory
 */
bool Store_write_cloud_portnum(uint16_t const *p_portnum)
{
    bool success=false;

    if(p_portnum)
    {
        if(eeprom_enable_write())
        {
            uint8_t buff[2];

            buff[0] = ALC_HI_BYTE(*p_portnum);
            buff[1] = ALC_LO_BYTE(*p_portnum);

            eeprom_write(EEPROM_CLOUD_PORTNUM_ADDR, buff, sizeof(buff));

            success = eeprom_last_op_success();
        }

        eeprom_disable_write();
    }

    if(success)
    {
        AlcLogger_log_info("Cloud port number updated in EEPROM");
        ModemCtrl_conf_has_been_changed();
    }
    else
    {
        AlcLogger_log_error("Failed to store Cloud port number in EEPROM");
    }

    return success;
}
/******************************************************************************/
bool Store_read_modem_apn(uint8_t *p_nstring, uint32_t maxlen)
{
    return eeprom_read_nstring(EEPROM_MODEM_APN_ADDR, p_nstring, maxlen);
}
/******************************************************************************/
bool Store_read_modem_apn_str(uint8_t *p_string, uint32_t maxlen)
{
    return eeprom_read_nstring_str(EEPROM_MODEM_APN_ADDR, p_string, maxlen);
}
/******************************************************************************/
bool Store_write_modem_apn(uint8_t const *p_nstring)
{
    bool success=false;

    if( is_valid_nstring(p_nstring) )
    {
        uint8_t len = p_nstring[0] & 0xFFU;

        if( len <= EEPROM_MODEM_APN_MAXLEN )
        {
            success = eeprom_write_nstring(EEPROM_MODEM_APN_ADDR, p_nstring);
        }
        else
        {
            AlcLogger_log_error("Modem APN is too long");
        }
    }

    if(success)
    {
        AlcLogger_log_info("Modem APN updated in EEPROM");
        ModemCtrl_conf_has_been_changed();
    }
    else
    {
        AlcLogger_log_error("Failed to store Modem APN in EEPROM");
    }

    return success;
}
/******************************************************************************/
bool Store_read_modem_username(uint8_t *p_nstring, uint32_t maxlen)
{
    return eeprom_read_nstring(EEPROM_MODEM_USERNAME_ADDR, p_nstring, maxlen);
}
/******************************************************************************/
bool Store_read_modem_username_str(uint8_t *p_string, uint32_t maxlen)
{
    return eeprom_read_nstring_str(EEPROM_MODEM_USERNAME_ADDR, p_string, maxlen);
}
/******************************************************************************/
bool Store_write_modem_username(uint8_t const *p_nstring)
{
    bool success=false;

    if( is_valid_nstring(p_nstring) )
    {
        uint8_t len = p_nstring[0] & 0xFFU;

        if( len <= EEPROM_MODEM_USERNAME_MAXLEN )
        {
            success = eeprom_write_nstring(EEPROM_MODEM_USERNAME_ADDR, p_nstring);
        }
        else
        {
            AlcLogger_log_error("Modem username is too long");
        }
    }

    if(success)
    {
        AlcLogger_log_info("Modem username updated in EEPROM");
        ModemCtrl_conf_has_been_changed();
    }
    else
    {
        AlcLogger_log_error("Failed to store Modem username in EEPROM");
    }

    return success;
}
/******************************************************************************/
bool Store_read_modem_password(uint8_t *p_nstring, uint32_t maxlen)
{
    return eeprom_read_nstring(EEPROM_MODEM_PASSWORD_ADDR, p_nstring, maxlen);
}
/******************************************************************************/
bool Store_read_modem_password_str(uint8_t *p_string, uint32_t maxlen)
{
    return eeprom_read_nstring_str(EEPROM_MODEM_PASSWORD_ADDR, p_string, maxlen);
}
/******************************************************************************/
bool Store_write_modem_password(uint8_t const *p_nstring)
{
    bool success=false;

    if( is_valid_nstring(p_nstring) )
    {
        uint8_t len = p_nstring[0] & 0xFFU;

        if( len <= EEPROM_MODEM_PASSWORD_MAXLEN )
        {
            success = eeprom_write_nstring(EEPROM_MODEM_PASSWORD_ADDR, p_nstring);
        }
        else
        {
            AlcLogger_log_error("Modem password is too long");
        }
    }

    if(success)
    {
        AlcLogger_log_info("Modem password updated in EEPROM");
        ModemCtrl_conf_has_been_changed();
    }
    else
    {
        AlcLogger_log_error("Failed to store Modem password in EEPROM");
    }

    return success;
}
/******************************************************************************/
bool Store_read_pan_ch(uint16_t *p_pan_ch)
{
    return read_u16_from_eeprom__(EEPROM_RADIO_PAN_CH_ADDR, p_pan_ch);
}
/******************************************************************************/
bool Store_write_pan_ch(uint16_t pan_ch)
{
    bool success = write_u16_to_eeprom__(EEPROM_RADIO_PAN_CH_ADDR, pan_ch);

    if(success)
    {
        AlcLogger_log_info("PAN Channel updated in EEPROM");
    }
    else
    {
        AlcLogger_log_error("Failed to store PAN Channel in EEPROM");
    }

    return success;
}
/******************************************************************************/
bool Store_read_pan_id(uint16_t *p_pan_id)
{
    return read_u16_from_eeprom__(EEPROM_RADIO_PAN_ID_ADDR, p_pan_id);
}
/******************************************************************************/
bool Store_write_pan_id(uint16_t pan_id)
{
    bool success = write_u16_to_eeprom__(EEPROM_RADIO_PAN_ID_ADDR, pan_id);

    if(success)
    {
        AlcLogger_log_info("PAN ID updated in EEPROM");
    }
    else
    {
        AlcLogger_log_error("Failed to store PAN ID in EEPROM");
    }

    return success;
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static bool read_u16_from_eeprom__(eeprom_addr_t address, uint16_t *p_value)
{
    bool success=false;

    if(p_value)
    {
        AlcStoreU16 buff;

        eeprom_read(address, (unsigned char*) &buff, sizeof(buff));

        if( eeprom_last_op_success() )
        {

            if( buff.crc16 == crc16_data((unsigned char const*) &buff.data, sizeof(buff.data), 0U) )
            {
                *p_value = buff.data.value;
                success = true;
            }
        }
    }

    return success;
}
/******************************************************************************/
static bool write_u16_to_eeprom__(eeprom_addr_t address, uint16_t value)
{
    bool success=false;

    if(eeprom_enable_write())
    {
        AlcStoreU16 buff;

        buff.data.value = value;
        buff.crc16      = crc16_data((unsigned char const*) &buff.data, sizeof(buff.data), 0U);

        eeprom_write(address, (unsigned char*) &buff, sizeof(buff));

        success = eeprom_last_op_success();
    }

    eeprom_disable_write();

    return success;
}
/******************************************************************************/
