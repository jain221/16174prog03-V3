/**
 * @file  gps_time_ctrl.h
 * @brief GPS Time Controller
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_GPS_GPS_TIME_CTRL_H_
#define SOURCE_INC_GPS_GPS_TIME_CTRL_H_




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


void GpsTimeCtrl_start_task(void const * argument);

void have_timestamp_from_server(uint32_t timestamp);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_GPS_GPS_TIME_CTRL_H_ */
