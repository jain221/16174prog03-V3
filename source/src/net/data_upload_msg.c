/**
 * @file  data_upload_msg.c
 * @brief Utility functions for uploading data to the cloud
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include <stdio.h>

#include "data_upload_msg.h"

#include "contiki.h"

#include "alc_ipaddr_snprintf.h"
#include "alc_string.h"
#include "bt/external_ble_interface.h"
#include "fw_version.h"
#include "gps_data.h"
#include "stm32_signature.h"




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




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static int32_t scale_accel__(uint8_t accel_fs, int16_t val);




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
void prepare_security_string_msg(char *dest, uint32_t len)
{
    if( (dest) && ( len > 43U ) )
    {
        snprintf(dest, len, "id,5b01d2a2-f2ad-11e6-bc64-92361f002671\r\n");
    }
}
/******************************************************************************/
void prepare_gateway_msg(char *dest, uint32_t len)
{
    if( (dest) && ( len > 8U ) )
    {
        /* Format: "gw,IP6ADDR,MA.MI.REL,LAT,LON"
         */
        GpsData gps_data;

        uip_ipaddr_t ipv6_address;
        getIPv6Address(ipv6_address.u8);

        if( !GpsData_retrieve(&gps_data, 100) )
        {
            gps_data.coord.lat = 0.0f;
            gps_data.coord.lon = 0.0f;
        }


        /* Initialise string */
        strncpy_safe(dest, "gw,", len);

        /* Append IP6ADDRESS */
        alc_ipaddr_snprintf(&dest[3], ( len - 3U ), &ipv6_address);

        uint32_t idx = strlen(dest);

        /* Append remaining data */
        snprintf(&dest[idx],
                 len,
                 ",%u.%u.%u,%0.6f,%0.6f\r\n",
                 FIRMWARE_MAJOR, FIRMWARE_MINOR, FIRMWARE_PATCH,
                 gps_data.coord.lat, gps_data.coord.lon
                 );
    }
}
/******************************************************************************/
void prepare_node_long_msg(char *dest, uint32_t len, SensorNode const *p_sensor_node)
{
    if( (dest) && ( len > 8U ) && (p_sensor_node) )
    {
        /* Format: "nd,IP6ADDR,STATUS,MA.MI.REL,STRATUM,LAT,LON,WAITING,CURRENT"
         */

        /* Initialise string */
        strncpy_safe(dest, "nd,", len);

        /* Append IP6ADDRESS */
        alc_ipaddr_snprintf(&dest[3], ( len - 3U ), &p_sensor_node->ipaddr);

        uint32_t idx = strlen(dest);

        /* Append remaining data */
        sprintf(&dest[idx], ",%s,%u.%u.%u,%u,%0.6f,%0.6f,%u,%u\r\n",
                SensorNode_get_status_string(p_sensor_node),
                p_sensor_node->fw_version[0],
                p_sensor_node->fw_version[1],
                p_sensor_node->fw_version[2],
                p_sensor_node->stratum,
                p_sensor_node->lat,
                p_sensor_node->lon,
                p_sensor_node->num_samples_waiting,
                p_sensor_node->bulb_current_ma_rms
                );
    }
}
/******************************************************************************/
void prepare_node_msg(char *dest, uint32_t len, SensorNode const *p_sensor_node)
{
    if( (dest) && ( len > 8U ) && (p_sensor_node) )
    {
        strncpy_safe(dest, "nd,", len);

        alc_ipaddr_snprintf(&dest[3], ( len - 3U ), &p_sensor_node->ipaddr);

        strcat(dest, "\r\n");
    }
}
/******************************************************************************/
void prepare_data_msg(char *dest, uint32_t len, struct SensorData *p_sensor_data)
{
    if( (dest) && ( len > 0U ) && (p_sensor_data) )
    {
        snprintf(dest,
                len,
                "da,%u.%02u,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
                p_sensor_data->ts_seconds,
                p_sensor_data->ts_hundreths,
                scale_accel__(p_sensor_data->accel_fs, p_sensor_data->accel_x),
                scale_accel__(p_sensor_data->accel_fs, p_sensor_data->accel_y),
                scale_accel__(p_sensor_data->accel_fs, p_sensor_data->accel_z),
                p_sensor_data->gyro_x,
                p_sensor_data->gyro_y,
                p_sensor_data->gyro_z,
                p_sensor_data->mag_x,
                p_sensor_data->mag_y,
                p_sensor_data->mag_z);
    }
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static int32_t scale_accel__(uint8_t accel_fs, int16_t val)
{
    int32_t ret_val = (int32_t) val;

    switch( accel_fs & 0x03U )
    {
    case 0U:
        /* 2g full-scale */
        ret_val /= 2;
        break;

    case 1U:
        /* 4g full-scale -- no need to do any adjustments here */
        break;

    case 2U:
        /* 8g full-scale */
        ret_val *= 2;
        break;

    case 3U:
        /* 16g full-scale */
        ret_val *= 4;
        break;
    }

    return ret_val;
}
/******************************************************************************/
