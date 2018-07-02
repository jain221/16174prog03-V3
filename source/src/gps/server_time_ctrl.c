/**
 * @file  server_time_ctrl.c
 * @brief Time Controller using messages from cloud server
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "gps_time_ctrl.h"

#include "alc_logger.h"
#include "alc_rtcc.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "modem_ctrl.h"
#include "task.h"

#define DEBUG DEBUG_NONE
#include "net-debug.h"


/* Set USING_SERVER_TIME_CTRL to non-zero value to enable getting the time from
 * the Cloud Server.
 */
#if USING_SERVER_TIME_CTRL




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

static AlcRtccStatus s_rtcc_status=ALC_RTCC_INVALID;




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
void GpsTimeCtrl_start_task(void const * argument)
{
    /* Initialise variables */
    s_rtcc_status = ALC_RTCC_INVALID;

    AlcRtcc_set_pps_duty_cycle(0U);
    AlcRtcc_stop_sync();

    osDelay(1000);

    PRINTF("server_time_task -- starting.\r\n");

    /* Task main loop */
    for(;;)
    {
        /* this task doesn't actually do anything */
        osDelay(10000);
    } /* end of for(;;) */
}
/******************************************************************************/
void have_timestamp_from_server(uint32_t timestamp)
{
    if( AlcRtcc_is_valid_timestamp(timestamp) )
    {
        AlcRtccData t1;

        /* get current time and calculate difference */
        AlcRtcc_get(&t1);
        uint32_t diff = ( timestamp - t1.seconds );

        if( diff != 0U )
        {
            /* time doesn't match our local time
             * Need to make adjustments...
             */
            AlcLogger_log_printf(ALC_LOGGER_INFO, "RTCC adjusting %d seconds", (int32_t) diff);
            AlcRtcc_adjust_seconds(diff);

            /* flag as in-sync */
            s_rtcc_status = ALC_RTCC_SYNC;
            AlcRtcc_set_pps_duty_cycle(10U);
        }
    }
}
/******************************************************************************/
AlcRtccStatus AlcRtcc_get_status(void)
{
    return s_rtcc_status;
}
/******************************************************************************/
char const * const AlcRtcc_get_status_string(void)
{
    switch(s_rtcc_status)
    {
    case ALC_RTCC_INVALID:
        return "INVALID";

    case ALC_RTCC_UNSYNC:
        return "UNSYNC";

    case ALC_RTCC_SYNC:
        return "SYNC";

    default:
        break;
    }

    return "UNDEFINED";
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/


#endif /* USING_SERVER_TIME_CTRL */
