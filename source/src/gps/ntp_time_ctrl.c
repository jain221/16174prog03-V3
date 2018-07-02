/**
 * @file  ntp_time_ctrl.c
 * @brief Time Controller using NTP over cellular network
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "gps_time_ctrl.h"

#include "alc_logger.h"
#include "alc_math.h"
#include "alc_rtcc.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "gps_data.h"
#include "modem_ctrl.h"
#include "modem_drv.h"
#include "task.h"

#define DEBUG DEBUG_NONE
#include "net-debug.h"


/* Set USING_NTP_TIME_CTRL to non-zero value to enable getting the time from the
 * Real-Time-Clock (RTC) on the Modem board, and using NTP to keep that clock in
 * sync.
 */
#if USING_NTP_TIME_CTRL


/*******************************************************************************
*                               LOCAL DEFINES
*******************************************************************************/

#define WAIT_DURATION_S             ( 60U * 10U )


#define LIST_OF_STATES(EXPAND_AS) \
                EXPAND_AS(MODE_MODEM_NOT_READY, "MODEM_NOT_READY"), \
                EXPAND_AS(MODE_CHECKING,        "CHECKING"), \
                EXPAND_AS(MODE_WAITING,         "WAITING")

#define EXPAND_AS_ENUM(id, str)             id
#define EXPAND_AS_STATUS_STRING(id, str)    str




/*******************************************************************************
*                               LOCAL CONSTANTS
*******************************************************************************/




/*******************************************************************************
*                               LOCAL DATA TYPES
*******************************************************************************/

typedef enum {
    LIST_OF_STATES(EXPAND_AS_ENUM),
    NUM_STATES
} OpMode;




/*******************************************************************************
*                               LOCAL TABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL GLOBAL VARIABLES
*******************************************************************************/

static OpMode s_operating_mode=MODE_MODEM_NOT_READY;
static AlcRtccStatus s_rtcc_status=ALC_RTCC_INVALID;
static clock_time_t s_time_of_last_diff_check=0U;




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static void set_status__(AlcRtccStatus new_status);
static bool get_time_diff__(uint32_t *p_diff);




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
AlcRtccStatus AlcRtcc_get_status(void)
{
    return s_rtcc_status;
}
/******************************************************************************/
char const * const AlcRtcc_get_status_string(void)
{
    static const char * p_strings[] = {
            LIST_OF_STATES(EXPAND_AS_STATUS_STRING)
    };

    if( s_operating_mode < NUM_STATES )
    {
        return p_strings[s_operating_mode];
    }

    return "INVALID";
}
/******************************************************************************/
void GpsTimeCtrl_start_task(void const * argument)
{
    GpsSentence gps_sentence;

    s_operating_mode = MODE_MODEM_NOT_READY;
    s_rtcc_status = ALC_RTCC_INVALID;
    AlcRtcc_set_pps_duty_cycle(0U);
    AlcRtcc_stop_sync();

    osDelay(1000);

    PRINTF("ntp_time_task -- starting.\r\n");

    for(;;)
    {
        switch(s_operating_mode)
        {
        case MODE_MODEM_NOT_READY:
            /****************************************************************
             * Modem is not ready
             ***************************************************************/
            PRINTF("ntp_time_task -- MODE_MODEM_NOT_READY -- waiting for modem\r\n");
            set_status__(ALC_RTCC_INVALID);
            AlcRtcc_set_pps_duty_cycle(0U);

            while( s_operating_mode == MODE_MODEM_NOT_READY )
            {
                if( ModemCtrl_modem_is_ready() )
                {
                    /* The Modem is ready */
                    s_operating_mode = MODE_CHECKING;
                    AlcRtcc_set_pps_duty_cycle(50U);
                }

                osDelay(5000);
            }
            break;


        case MODE_CHECKING:
            /****************************************************************
             *                             CHECKING
             *
             * Checking the local RTC clock to the RTC on the Modem...
             * and making adjustments to the local RTC if necessary.
             *
             ***************************************************************/
            PRINTF("ntp_time_task -- MODE_CHECKING\r\n");

            while( s_operating_mode == MODE_CHECKING )
            {
                uint32_t diff;

                if( !ModemCtrl_modem_is_ready() )
                {
                    /* lost Modem */
                    s_operating_mode = MODE_MODEM_NOT_READY;
                }
                else if( !get_time_diff__(&diff) )
                {
                    /* failed to get the Modem RTC time -- try again in a little while */
                    PRINTF("ntp_time_task -- failed to get the time -- try again in a little while\r\n");
                    osDelay(5000);
                }
                else if(diff != 0U)
                {
                    /* got Modem RTC time, but it doesn't match our local time
                     * Need to make adjustments...
                     */
                    AlcLogger_log_printf(ALC_LOGGER_INFO, "RTCC adjusting %+d seconds", diff);
                    AlcRtcc_adjust_seconds(diff);

                    if( abs_i32(diff) > 10 )
                    {
                        /* diff was greater than 10 seconds, so wait 5 seconds
                         * and then compare RTC clocks again.
                         */
                        osDelay(5000);
                        set_status__(ALC_RTCC_UNSYNC);
                        AlcRtcc_set_pps_duty_cycle(50U);
                    }
                    else
                    {
                        /* diff was less than 10 seconds, so go to WAITING state */
                        s_operating_mode = MODE_WAITING;
                    }
                }
                else
                {
                    /* got gps time and it matches our local time */
                    PRINTF("ntp_time_task -- got time from Modem -- both match\r\n");
                    s_operating_mode = MODE_WAITING;
                }
            }
            break;


        case MODE_WAITING:
            /****************************************************************
             *                             WAITING
             ***************************************************************/
            {
                uint32_t waited_for_s=0U;

                PRINTF("ntp_time_task -- MODE_WAITING\r\n");
                set_status__(ALC_RTCC_SYNC);
                AlcRtcc_set_pps_duty_cycle(10U);

                while( s_operating_mode == MODE_WAITING )
                {
                    /* Sleep for 10 seconds */
                    osDelay(10000);
                    waited_for_s += 10U;

                    /* do checks */
                    if( !ModemCtrl_modem_is_ready() )
                    {
                        /* lost Modem */
                        s_operating_mode = MODE_MODEM_NOT_READY;
                    }
                    else if( waited_for_s >= WAIT_DURATION_S )
                    {
                        /* Been waiting for 10 minutes...
                         * Time to check the check the local RTC against the
                         * RTC on the Modem.
                         */
                        s_operating_mode = MODE_CHECKING;
                    }
                    else
                    {
                        /* do nothing here */
                    }
                }
            }
            break;


        default:
            /* should never get here */
            s_operating_mode = MODE_MODEM_NOT_READY;
            break;
        } /* end of switch() */
    } /* end of for(;;) */
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static void set_status__(AlcRtccStatus new_status)
{
    if( s_rtcc_status != new_status )
    {
        if( new_status == ALC_RTCC_SYNC )
        {
            /* entering synchronised state */
            AlcLogger_log_info("RTCC is synchronised to Modem RTC");
        }
        else if( s_rtcc_status == ALC_RTCC_SYNC )
        {
            /* exiting synchronised state */
            AlcLogger_log_info("RTCC lost synchronisation with Modem RTC");
        }
        else
        {
            /* other transition -- do nothing here */
        }


        s_rtcc_status = new_status;
    }
}
/******************************************************************************/
static bool get_time_diff__(uint32_t *p_diff)
{
    uint32_t rtc;
    bool success=false;
    AlcRtccData t1;


    /* To reliably read the time from the GPS module we need to limit
     * reading it to the first 1/4 of the second. In the second half of the
     * second the module seems to report the next second.
     */
    AlcRtcc_get(&t1);
    while( t1.subseconds >= 0x4000U )
    {
        osDelay(100);
        AlcRtcc_get(&t1);
    }


    if( Modem_get_rtc(&rtc) )
    {
        /* Got time from GPS module */
        AlcRtccData t2;

        AlcRtcc_get(&t2);

#if 0
        PRINTF("get_time_diff__():\r\n");
        PRINTF("  t1   = %lu.%04x\r\n", t1.seconds, t1.subseconds);
        PRINTF("  t2   = %lu.%04x\r\n", t2.seconds, t2.subseconds);
        PRINTF("  rtc  = %lu\r\n", rtc);
        PRINTF("  diff = %ld\r\n", ( rtc - t2.seconds ));
#endif

        if( t1.seconds == t2.seconds )
        {
            /* success -- got the time from the GPS module and the RTCC
             * seconds did not incremented while we were doing it.
             */
            success = true;

            if(p_diff)
            {
                *p_diff = ( rtc - t2.seconds );
            }
        }
    }

    return success;
}
/******************************************************************************/


#endif /* USING_NTP_TIME_CTRL */
