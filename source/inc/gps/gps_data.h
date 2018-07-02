/**
 * @file  gps_data.h
 * @brief Store for GPS location data
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_GPS_GPS_DATA_H_
#define SOURCE_INC_GPS_GPS_DATA_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "clock.h"




/*******************************************************************************
*                               DEFAULT CONFIGURATION
*******************************************************************************/




/*******************************************************************************
*                               DEFINES
*******************************************************************************/




/*******************************************************************************
*                               DATA TYPES
*******************************************************************************/

typedef struct {
    double lat;     /* latitude */
    double lon;     /* longitude */
} GpsPos;


typedef struct {
    uint32_t    gps_status;
    uint32_t    fix_status;
    struct tm   tm;
    GpsPos      coord;
} GpsSentence;


typedef struct {
    GpsPos       coord;         /* GPS position */
    clock_time_t last_update;   /* time of last update */
} GpsData;




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


bool GpsData_store(GpsSentence const *p_data, uint32_t timeout_ms);
bool GpsData_retrieve(GpsData *p_data, uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_GPS_GPS_DATA_H_ */
