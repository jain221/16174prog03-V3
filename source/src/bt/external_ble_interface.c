/**
 * @file  external_ble_interface.c
 * @brief Functions used by Bluetooth library to access program parameters
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "external_ble_interface.h"

#include "alc_bluetooth_mac.h"
#include "alc_logger.h"
#include "alc_radio_setup.h"
#include "eeprom_arch.h"
#include "fw_version.h"
#include "gps_data.h"
#include "net/ipv6/uip-ds6.h"
#include "nv_settings.h"
#include "stm32_signature.h"


#define DEBUG DEBUG_NONE
#include "net-debug.h"
#include "uip-debug.h"




/*******************************************************************************
*                               LOCAL DEFINES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL CONSTANTS
*******************************************************************************/




/*******************************************************************************
*                               LOCAL DATA TYPES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL TABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL GLOBAL VARIABLES
*******************************************************************************/

uint8_t MACaddress[6];
uint8_t FirmwareVersion[3];




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/

static bool search_ipv6_address__(uint8_t *p_dest_ipv6_address, uint8_t state, uint16_t prefix);




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
/** @brief Get the IPv6 address
 *
 * @param p_ipv6_address    Pointer to uint8_t[16]
 */
void getIPv6Address(uint8_t *p_ipv6_address)
{
    if(p_ipv6_address)
    {
        PRINTF("  Searching for IPv6 address...\r\n");

        if( search_ipv6_address__(p_ipv6_address, ADDR_PREFERRED, PREFIX_ADDR0) )
        {
            PRINTF("success -- found preferred address with our prefix\r\n");
        }
        else if( search_ipv6_address__(p_ipv6_address, ADDR_TENTATIVE, PREFIX_ADDR0) )
        {
            PRINTF("success -- found tentative address with our prefix\r\n");
        }
        else if( search_ipv6_address__(p_ipv6_address, ADDR_PREFERRED, 0) )
        {
            PRINTF("success -- found preferred address\r\n");
        }
        else if( search_ipv6_address__(p_ipv6_address, ADDR_TENTATIVE, 0) )
        {
            PRINTF("success -- found tentative address\r\n");
        }
        else
        {
            PRINTF("failed -- did not find address!\r\n");
            memset(p_ipv6_address, 0, 16);
        }
    }
}
/******************************************************************************/

#define MAKE_U64(hi32,lo32)         ( ( (uint64_t) (hi32) << 32 ) | ( (uint64_t) (lo32) & 0xFFFFFFFFL ) )


/** @brief Get the 48-bit MAC address
 *
 * @param p_mac_address    Pointer to uint8_t[6]
 */
void getMACAddress(uint8_t *p_mac_address)
{
    get_bluetooth_mac48(p_mac_address, 6);
}
/******************************************************************************/
/** @brief get the firmware version
 *
 * @param p_fw_version    Pointer to uint8_t[3] - Major, minor, patch
 */
void getFirmwareVersion(uint8_t * p_fw_version)
{
    if(p_fw_version)
    {
        p_fw_version[0] = FIRMWARE_MAJOR;
        p_fw_version[1] = FIRMWARE_MINOR;
        p_fw_version[2] = FIRMWARE_PATCH;
    }
}
/******************************************************************************/
/** @brief Get lamp state
 *
 * @param p_lamp_state  0 = Off,  1 = On
 */
void getLampState(uint8_t * p_lamp_state)
{
    if(p_lamp_state)
    {
        // Always return 0 (off) -- we don't have a lamp
        *p_lamp_state = 0U;
    }
}
/******************************************************************************/
void setLampState(uint8_t lamp_state)
{
    // Ignore -- we don't have a lamp
}
/******************************************************************************/
/** @brief Get lamp current (in mA rms)
 *
 * @param p_lamp_current_ma     pointer to result
 */
void getLampCurrent(uint32_t *p_lamp_current_ma)
{
    if(p_lamp_current_ma)
    {
        // Always return 0 (off) -- we don't have a lamp
        *p_lamp_current_ma = 0;
    }
}
/******************************************************************************/
void getGPS(GPScoords_t* p_gps_location)
{
    if(p_gps_location)
    {
        GpsData gps_data;

        if( GpsData_retrieve(&gps_data, 100) )
        {
            /* read from storage ok */
            p_gps_location->longitude = gps_data.coord.lon;
            p_gps_location->latitude  = gps_data.coord.lat;
        }
        else
        {
            /* failed to read from storage -- set everything to 0 */
            p_gps_location->longitude = 0.0f;
            p_gps_location->latitude  = 0.0f;
        }
    }
}
/******************************************************************************/
void setGPSLocation(GPScoords_t const* p_gps_location)
{
    if(p_gps_location)
    {
        // don't set gps location
    }
}
/******************************************************************************/
// (4 bytes)
void GetGatewayIP(uint8_t* GatewayIP)
{
    Store_read_cloud_ipv4(GatewayIP);
}
/******************************************************************************/
// (4 bytes)
void setGatewayIP(uint8_t const* GatewayIP)
{
    Store_write_cloud_ipv4(GatewayIP);
}
/******************************************************************************/
void GetGatewayPort(uint16_t* GateawayPort)
{
    Store_read_cloud_portnum(GateawayPort);
}
/******************************************************************************/
void setGatewayPort(uint16_t const* GateawayPort)
{
    Store_write_cloud_portnum(GateawayPort);
}
/******************************************************************************/
// (20 bytes – 1st is length)
void GetGatewayAPN(uint8_t* GatewayAPN)
{
    Store_read_modem_apn(GatewayAPN, 19);
}
/******************************************************************************/
// (20 bytes – 1st is length)
void setGatewayAPN(uint8_t const* GatewayAPN)
{
    Store_write_modem_apn(GatewayAPN);
}
/******************************************************************************/
// (20 bytes – 1st is length)
void GetGatewayUsername(uint8_t* GatewayUsername)
{
    Store_read_modem_username(GatewayUsername, 19);
}
/******************************************************************************/
// (20 bytes – 1st is length)
void setGatewayUsername(uint8_t const* GatewayUsername)
{
    Store_write_modem_username(GatewayUsername);
}
/******************************************************************************/
// (20 bytes – 1st is length)
void GetGatewayPassword(uint8_t* GatewayPassword)
{
    Store_read_modem_password(GatewayPassword, 19);
}
/******************************************************************************/
// (20 bytes – 1st is length)
void setGatewayPassword(uint8_t const* GatewayPassword)
{
    Store_write_modem_password(GatewayPassword);
}
/******************************************************************************/
void getPanChannel(uint16_t *p_pan_channel)
{
    if(p_pan_channel)
    {
        *p_pan_channel = AlcRadioSetup_get_pan_channel();
    }
}
/******************************************************************************/
void setPanChannel(uint16_t const *p_pan_channel)
{
    if(p_pan_channel)
    {
        AlcRadioSetup_set_pan_channel(*p_pan_channel);
    }
}
/******************************************************************************/
void getPanId(uint16_t *p_pan_id)
{
    if(p_pan_id)
    {
        *p_pan_id = AlcRadioSetup_get_pan_id();
    }
}
/******************************************************************************/
void setPanId(uint16_t const *p_pan_id)
{
    if(p_pan_id)
    {
        AlcRadioSetup_set_pan_id(*p_pan_id);
    }
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static bool search_ipv6_address__(uint8_t *p_dest_ipv6_address, uint8_t state, uint16_t prefix)
{
    if(p_dest_ipv6_address)
    {
        for(int ii = 0; ii < UIP_DS6_ADDR_NB; ii++ )
        {
            if(
                    ( uip_ds6_if.addr_list[ii].isused ) &&
                    ( uip_ds6_if.addr_list[ii].state == state )
            )
            {
                if(
                        ( prefix == 0U ) ||
                        ( prefix == UIP_HTONS(uip_ds6_if.addr_list[ii].ipaddr.u16[0] ) )
                )
                {
                    PRINTF("    IP = %u, ", uip_ds6_if.addr_list[ii].state);
                    PRINT6ADDR(&uip_ds6_if.addr_list[ii].ipaddr);
                    PRINTF("\r\n");

                    /* copy address */
                    memcpy(p_dest_ipv6_address, uip_ds6_if.addr_list[ii].ipaddr.u8, sizeof(uip_ds6_if.addr_list[ii].ipaddr));

                    return true;
                }
            }
        }
    }

    return false;
}
/******************************************************************************/
