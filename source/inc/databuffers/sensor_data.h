/**
 * @file  sensor_data.h
 * @brief A struct for holding pole data to be streamed to the server.
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_DATABUFFERS_SENSOR_DATA_H_
#define SOURCE_INC_DATABUFFERS_SENSOR_DATA_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
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

struct SensorData {
    struct SensorData *p_prev;  /* The previous object in the linked-list. (NULL if none) */
    struct SensorData *p_next;  /* The next object in the linked-list. (NULL if no more) */
    uint32_t seq32;             /* sequence number */
    uint32_t ts_seconds;        /* UTC timestamp seconds */
    uint8_t  ts_hundreths;      /* UTC timestamp hundreth's of a seconds */
    int16_t  accel_x;
    int16_t  accel_y;
    int16_t  accel_z;
    int16_t  gyro_x;
    int16_t  gyro_y;
    int16_t  gyro_z;
    int16_t  mag_x;
    int16_t  mag_y;
    int16_t  mag_z;
    uint8_t  accel_fs;          /* Accelerometer full-scale */
};




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




#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_DATABUFFERS_SENSOR_DATA_H_ */
