/**
 * @file  nv_settings.h
 * @brief Non-Volatile Settings -- stored in EEPROM
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_STORAGE_NV_SETTINGS_H_
#define SOURCE_INC_STORAGE_NV_SETTINGS_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include <stdbool.h>
#include <stdint.h>




/*******************************************************************************
*                               DEFAULT CONFIGURATION
*******************************************************************************/




/*******************************************************************************
*                               DEFINES
*******************************************************************************/




/*******************************************************************************
*                               DATA TYPES
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL VARIABLES
*******************************************************************************/




/*******************************************************************************
*                               MACRO's
*******************************************************************************/




/*******************************************************************************
*                               FUNCTION PROTOTYPES
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


bool Store_read_cloud_ipv4(uint8_t *p_buff4);
bool Store_write_cloud_ipv4(uint8_t const *p_buff4);

bool Store_read_cloud_portnum(uint16_t *p_portnum);
bool Store_write_cloud_portnum(uint16_t const *p_portnum);

bool Store_read_modem_apn(uint8_t *p_nstring, uint32_t maxlen);
bool Store_read_modem_apn_str(uint8_t *p_string, uint32_t maxlen);
bool Store_write_modem_apn(uint8_t const *p_nstring);

bool Store_read_modem_username(uint8_t *p_nstring, uint32_t maxlen);
bool Store_read_modem_username_str(uint8_t *p_string, uint32_t maxlen);
bool Store_write_modem_username(uint8_t const *p_nstring);

bool Store_read_modem_password(uint8_t *p_nstring, uint32_t maxlen);
bool Store_read_modem_password_str(uint8_t *p_string, uint32_t maxlen);
bool Store_write_modem_password(uint8_t const *p_nstring);

bool Store_read_pan_ch(uint16_t *p_pan_ch);
bool Store_write_pan_ch(uint16_t pan_ch);

bool Store_read_pan_id(uint16_t *p_pan_ch);
bool Store_write_pan_id(uint16_t pan_ch);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_STORAGE_NV_SETTINGS_H_ */
