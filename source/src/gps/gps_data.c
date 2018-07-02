/**
 * @file  gps_data.c
 * @brief Store for GPS location data
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "gps_data.h"

#include "cmsis_os.h"


#define DEBUG_STREAM 0
#if DEBUG_STREAM
#include <stdio.h>
#endif




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

extern osMutexId g_gps_data_mutexHandle;

static GpsData s_gps_data;




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
bool GpsData_store(GpsSentence const *p_data, uint32_t timeout_ms)
{
    bool success = false;


    if( p_data )
    {
        /*
         * Using mutex to protect GPS data from access by multiple threads
         */
        if( osMutexWait(g_gps_data_mutexHandle, timeout_ms) == osOK )
        {

            if(
                    ( p_data->gps_status != 0 ) &&
                    ( p_data->fix_status != 0 )
            )
            {
                success = true;
                s_gps_data.coord       = p_data->coord;
                s_gps_data.last_update = clock_time();
            }

            /* release mutex */
            osMutexRelease(g_gps_data_mutexHandle);

#if DEBUG_STREAM
            printf("GPS position updated: lat=%06f, lon=%06f\r\n", s_gps_data.coord.lat, s_gps_data.coord.lon);
#endif
        }
    }

    return success;
}
/******************************************************************************/
bool GpsData_retrieve(GpsData *p_data, uint32_t timeout_ms)
{
    bool success = false;

    if( p_data )
    {
        /*
         * Using mutex to protect GPS data from access by multiple threads
         */
        if( osMutexWait(g_gps_data_mutexHandle, timeout_ms) == osOK )
        {
            success = true;

            *p_data = s_gps_data;

            /* release mutex */
            osMutexRelease(g_gps_data_mutexHandle);
        }
    }

    return success;
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/
