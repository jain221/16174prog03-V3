/**
 * @file  gps_time_ctrl.c
 * @brief GPS Time Controller
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "gps_time_ctrl.h"

#include "alc_logger.h"
#include "alc_rtcc.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "gps_data.h"
#include "modem_ctrl.h"
#include "task.h"

#define DEBUG DEBUG_NONE
#include "net-debug.h"


/* Set USING_GPS_TIME_CTRL to non-zero value to enable getting the time from the
 * GPS module on the Modem board.
 */
#if USING_GPS_TIME_CTRL


/*******************************************************************************
*                               LOCAL DEFINES
*******************************************************************************/

#define PPS_TIMEOUT_MS              10000LU
#define RECHECK_CLOCK_SYNC_MS       60000LU




/*******************************************************************************
*                               LOCAL CONSTANTS
*******************************************************************************/




/*******************************************************************************
*                               LOCAL DATA TYPES
*******************************************************************************/

typedef enum {
    MODE_NOSIGNAL=0,    /* no PPS signal from GPS */
    MODE_UNSYNC,        /* Got PPS signal, but not synchronised with it */
    MODE_SYNC_NO_TIME,  /* Got PPS signal and synchronised with it, but time is not set */
    MODE_SYNC           /* Got PPS signal and synchronised with it and time is set */
} OpMode;




/*******************************************************************************
*                               LOCAL TABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL GLOBAL VARIABLES
*******************************************************************************/

static OpMode s_operating_mode=MODE_NOSIGNAL;
static AlcRtccStatus s_rtcc_status=ALC_RTCC_INVALID;
static clock_time_t s_time_of_last_diff_check=0U;




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static bool have_pps_signal(void);
static bool get_time_diff(GpsSentence *p_inf, uint32_t *p_diff);
static void store_gps_data(GpsSentence *p_inf);
static void set_status(AlcRtccStatus new_status);




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
            "NOSIGNAL",
            "UNSYNC",
            "SYNC_NOTIME",
            "SYNC"
    };

    if( s_operating_mode < 4 )
    {
        return p_strings[s_operating_mode];
    }

    return "INVALID";
}
/******************************************************************************/
void GpsTimeCtrl_start_task(void const * argument)
{
    GpsSentence gps_sentence;

    s_operating_mode = MODE_NOSIGNAL;
    s_rtcc_status = ALC_RTCC_INVALID;
    AlcRtcc_set_pps_duty_cycle(90U);
    AlcRtcc_start_sync();

    osDelay(1000);

    PRINTF("gps_time_task -- starting.\r\n");

    for(;;)
    {
        switch(s_operating_mode)
        {
        case MODE_NOSIGNAL:
            /****************************************************************
             * No PPS signal from GPS
             ***************************************************************/
            PRINTF("gps_time_task -- MODE_NOSIGNAL -- no signal from GPS\r\n");
            set_status(ALC_RTCC_INVALID);
            AlcRtcc_set_pps_duty_cycle(90U);

            while( s_operating_mode == MODE_NOSIGNAL )
            {
                if( have_pps_signal() )
                {
                    /* got PPS signal */
                    s_operating_mode = MODE_UNSYNC;
                }
                else
                {
                    osDelay(1000);
                }
            }

            break;

        case MODE_UNSYNC:
            /****************************************************************
             * Got PPS signal, but not synchronised with it
             ***************************************************************/
            PRINTF("gps_time_task -- MODE_UNSYNC -- RTCC is not in sync!\r\n");
            set_status(ALC_RTCC_UNSYNC);
            AlcRtcc_set_pps_duty_cycle(50U);

            while( s_operating_mode == MODE_UNSYNC )
            {
                if(!have_pps_signal())
                {
                    /* lost PPS signal */
                    s_operating_mode = MODE_NOSIGNAL;
                }
                else if( AlcRtcc_is_in_sync() )
                {
                    /* now in sync */
                    s_operating_mode = MODE_SYNC_NO_TIME;
                }
                else
                {
                    osDelay(500);
                }
            }
            break;

        case MODE_SYNC_NO_TIME:
            /****************************************************************
             * Got PPS signal and synchronised with it, but time is not set
             ***************************************************************/
            {
                PRINTF("gps_time_task -- MODE_SYNC_NO_TIME -- RTCC is not in sync!\r\n");
                set_status(ALC_RTCC_UNSYNC);
                AlcRtcc_set_pps_duty_cycle(50U);

                uint32_t diff;

                while( s_operating_mode == MODE_SYNC_NO_TIME )
                {
                    if( !have_pps_signal() )
                    {
                        /* lost PPS signal */
                        s_operating_mode = MODE_NOSIGNAL;
                    }
                    else if( !AlcRtcc_is_in_sync() )
                    {
                        /* lost synchronisation */
                        s_operating_mode = MODE_UNSYNC;
                    }
                    else if( !get_time_diff(&gps_sentence, &diff) )
                    {
                        /* failed to get the time -- try again in a little while */
                        PRINTF("gps_time_task -- failed to get the time -- try again in a little while\r\n");
                        osDelay(5000);
                    }
                    else if(diff != 0U)
                    {
                        /* got gps time but it doesn't match our local time */
                        PRINTF("gps_time_task -- got gps time but it doesn't match our local time (diff=%d)\r\n", diff);
                        AlcRtcc_adjust_seconds(diff);
                        osDelay(5000);
                    }
                    else
                    {
                        /* got gps time and it matches our local time */
                        PRINTF("gps_time_task -- got gps time and it matches our local timer\n");
                        s_operating_mode = MODE_SYNC;
                        /* got time */
                    }
                }
            }
            break;

        case MODE_SYNC:
            /****************************************************************
             * Got PPS signal and synchronised with it and time is set
             ***************************************************************/
            {
                PRINTF("gps_time_task -- MODE_SYNC -- RTCC is synchronised\r\n");
                set_status(ALC_RTCC_SYNC);
                AlcRtcc_set_pps_duty_cycle(10U);

                uint32_t diff;

                s_time_of_last_diff_check = clock_time();

                while( s_operating_mode == MODE_SYNC )
                {
                    if( !have_pps_signal() )
                    {
                        /* lost PPS signal */
                        s_operating_mode = MODE_NOSIGNAL;
                    }
                    else if( !AlcRtcc_is_in_sync() )
                    {
                        /* lost synchronisation */
                        s_operating_mode = MODE_UNSYNC;
                    }
                    else if( ( clock_time() - s_time_of_last_diff_check ) < RECHECK_CLOCK_SYNC_MS )
                    {
                        /* The time since the last check on time is less than
                         * RECHECK_CLOCK_SYNC_MS -- so nothing to do here
                         */
                        osDelay(2000);
                    }
                    else if( !get_time_diff(&gps_sentence, &diff) )
                    {
                        /* failed to get the time -- try again in a little while */
                        PRINTF("gps_time_task -- failed to get the time -- try again in a little while\r\n");
                        osDelay(5000);
                    }
                    else if(diff != 0U)
                    {
                        /* got gps time but it doesn't match our local time anymore */
                        PRINTF("gps_time_task -- got gps time but it doesn't match our local time anymore (diff=%d)\r\n", diff);
                        s_operating_mode = MODE_SYNC_NO_TIME;
                        store_gps_data(&gps_sentence);
                    }
                    else
                    {
                        /* got gps time and it still matches our local time
                         * record the time of this diff check
                         */
                        s_time_of_last_diff_check = clock_time();
                        store_gps_data(&gps_sentence);
                        osDelay(2000);
                    }
                }
            }
        break;

        default:
            /* should never get here */
            s_operating_mode = MODE_NOSIGNAL;
            break;
        } /* end of switch() */
    } /* end of for(;;) */
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static void set_status(AlcRtccStatus new_status)
{
    if( s_rtcc_status != new_status )
    {
        if( new_status == ALC_RTCC_SYNC )
        {
            /* entering synchronised state */
            AlcLogger_log_info("RTCC is synchronised to GPS");
        }
        else if( s_rtcc_status == ALC_RTCC_SYNC )
        {
            /* exiting synchronised state */
            AlcLogger_log_info("RTCC lost synchronisation with GPS");
        }
        else
        {
            /* other transition -- do nothing here */
        }


        s_rtcc_status = new_status;
    }
}
/******************************************************************************/
static bool have_pps_signal(void)
{
    uint32_t last_pps_time = AlcRtcc_get_last_pps_time();

    if(
            ( last_pps_time == 0U ) &&
            ( ( clock_time() - last_pps_time ) > PPS_TIMEOUT_MS )
    )
    {
        return false;
    }

    return true;
}
/******************************************************************************/
static bool get_time_diff(GpsSentence *p_inf, uint32_t *p_diff)
{
    bool success=false;

    /* Don't try to get the time if the gps in the modem is not ready. */
    if( ModemCtrl_gps_is_ready() )
    {
        AlcRtccData t1;
        uint32_t gps_t;

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


        if( ModemCtrl_get_gps_info(p_inf, &gps_t) )
        {
            /* Got time from GPS module */
            AlcRtccData t2;

            AlcRtcc_get(&t2);

#if 0
            PRINTF("get_time_diff():\r\n");
            PRINTF("  t1   = %lu.%04x\r\n", t1.seconds, t1.subseconds);
            PRINTF("  t2   = %lu.%04x\r\n", t2.seconds, t2.subseconds);
            PRINTF("  gps  = %lu\r\n", gps_t);
            PRINTF("  diff = %ld\r\n", ( gps_t - t2.seconds ));
            PRINTF("  pos  = %f, %f\r\n", p_inf->coord.lat, p_inf->coord.lon);
#endif

            if( t1.seconds == t2.seconds )
            {
                /* success -- got the time from the GPS module and the RTCC
                 * seconds did not incremented while we were doing it.
                 */
                success = true;

                if(p_diff)
                {
                    *p_diff = ( gps_t - t2.seconds );
                }
            }
        }
    }

    return success;
}
/******************************************************************************/
static void store_gps_data(GpsSentence *p_inf)
{
    if(p_inf)
    {
        /* Send the data to the GPS datastore */
        (void) GpsData_store(p_inf, 100);
    }
}
/******************************************************************************/


#endif /* USING_GPS_TIME_CTRL */
