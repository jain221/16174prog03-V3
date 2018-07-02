/**
 * @file  modem_ctrl_wifi.c
 * @brief Modem controller (rtos task)
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "modem_ctrl.h"

#include "alc_logger.h"
#include "alc_nmea_utils.h"
#include "alc_string.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "modem_drv.h"
#include "stm32xxxx_hal.h"
#include "stm32xxxx_hal_cortex.h"
#include "task.h"


#define DEBUG DEBUG_NONE
#include "net-debug.h"

#define DEBUG_GPS_TIME 0




/*******************************************************************************
*                               LOCAL DEFINES
*******************************************************************************/

/* how long to delay configuration-changed event */
#define DELAY_CONF_CHANGED_EV_MS        ( 120LU * 1000LU )




/*******************************************************************************
*                               LOCAL CONSTANTS
*******************************************************************************/




/*******************************************************************************
*                               LOCAL DATA TYPES
*******************************************************************************/

typedef enum {
    ST_INITIALISING=0,
#if MODEM_HAS_GPS
    ST_ENABLE_GPS,
#endif
    ST_JOINING_NETWORK,
    ST_CONFIGURE_LINK,
    ST_RUNNING,
    ST_SHUTDOWN
} ModemState;




/*******************************************************************************
*                               LOCAL TABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL GLOBAL VARIABLES
*******************************************************************************/

static ModemState s_state=ST_INITIALISING;
static bool s_gps_ready=false;
static struct {
    volatile uint32_t changed_at;
    volatile bool     has_changed;
} s_conf_changed = {0U};
static volatile bool s_restart_modem=false;




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static bool begin(void);
static void enable_gps(void);
static bool registered_to_network(void);
static bool configure_link(void);
static bool decode_cgnsinf_sentence(char const *str);
static void check_conf__(void);
static bool is_connected__(void);




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
uint32_t ModemCtrl_get_state(void)
{
    return (uint32_t) s_state;
}
/******************************************************************************/
bool ModemCtrl_modem_is_ready(void)
{
    return ( s_state == ST_RUNNING );
}
/******************************************************************************/
bool ModemCtrl_gps_is_ready(void)
{
    return s_gps_ready;
}
/******************************************************************************/

static char s_cgnsinf_sentence[120];

static bool decode_cgnsinf_sentence(char const *str)
{
    if( strncmp(str, "+CGNSINF: ", 10) == 0 )
    {
        strncpy_safe(s_cgnsinf_sentence, str, sizeof(s_cgnsinf_sentence));
    }

    return true;
}
/******************************************************************************/
bool ModemCtrl_get_gps_info(GpsSentence *p_inf, uint32_t *p_timestamp)
{
    bool success=false;

    s_cgnsinf_sentence[0] = '\0';

    if(p_inf)
    {
        if( Modem_run_command_ex("AT+CGNSINF", SEARCH_OK | SEARCH_ERROR, &decode_cgnsinf_sentence, NULL, 0U, 200) )
        {
#if DEBUG_GPS_TIME
            PRINTF("  >>>>%s<<<< %u\r\n", s_cgnsinf_sentence, strlen(s_cgnsinf_sentence));
#endif

            if( minmea_utils_parse_inf(s_cgnsinf_sentence, p_inf) )
            {
#if DEBUG_GPS_TIME
                PRINTF("    gps_status = %u\r\n", p_inf->gps_status);
                PRINTF("    fix_status = %u\r\n", p_inf->fix_status);
                PRINTF("    date       = %02u/%02u/%04u\r\n", p_inf->tm.tm_mday, p_inf->tm.tm_mon + 1, p_inf->tm.tm_year + 1900);
                PRINTF("    time       = %02u:%02u:%02u\r\n", p_inf->tm.tm_hour, p_inf->tm.tm_min, p_inf->tm.tm_sec);
#endif

                if(p_timestamp)
                {
                    if( p_inf->gps_status != 0U )
                    {
                        /* GPS info is valid */
                        *p_timestamp = timegm(&p_inf->tm); /* See README.md if your system lacks timegm(). */
                    }
                    else
                    {
                        /* GPS info is not valid */
                        *p_timestamp = 0U;
                    }

#if DEBUG_GPS_TIME
                    PRINTF("    timestamp  = %lu\r\n", *p_timestamp);
#endif
                }

#if DEBUG_GPS_TIME
                PRINTF("    pos        = %f, %f\r\n", p_inf->coord.lat, p_inf->coord.lon);
#endif

                success = true;
            }
        }
    }

    return success;
}
/******************************************************************************/
void ModemCtrl_restart_modem(void)
{
    AlcLogger_log_info("Requesting restart Modem");

    s_restart_modem = true;
}
/******************************************************************************/
void ModemCtrl_conf_has_been_changed(void)
{
    AlcLogger_log_info("ModemCtrl_conf_has_been_changed()");

    s_conf_changed.has_changed = false;
    s_conf_changed.changed_at  = HAL_GetTick();
    s_conf_changed.has_changed = true;
}
/******************************************************************************/
bool ModemCtrl_check_conf_has_changed(void)
{
    if(
            ( s_conf_changed.has_changed ) &&
            ( ( HAL_GetTick() - s_conf_changed.changed_at ) >= DELAY_CONF_CHANGED_EV_MS )
    )
    {
        return true;
    }

    return false;
}
/******************************************************************************/
void ModemCtrl_start_task(void const * argument)
{
    ModemState last_state;
    uint32_t num_attempts=0U;

    s_state     = ST_INITIALISING;
    s_gps_ready = false;

    s_restart_modem = false;
    s_conf_changed.has_changed = false;


    osDelay(5000);

    PRINTF("MODEM TASK -- starting\r\n");

    last_state = s_state;
    for(;;)
    {
        switch(s_state)
        {
        case ST_INITIALISING:
            PRINTF("ModemCtrl -- initialising\r\n");
            AlcLogger_log_info("Resetting the modem");
            s_restart_modem = false;
            s_conf_changed.has_changed = false;
            num_attempts = 0U;
            while( s_state == ST_INITIALISING )
            {
                check_conf__();

                if(begin())
                {
                    PRINTF("ModemCtrl -- found modem\r\n");
#if MODEM_HAS_GPS
                    s_state = ST_ENABLE_GPS;
#else
                    s_state = ST_JOINING_NETWORK;
#endif
                }
                else
                {
                    PRINTF("ModemCtrl -- failed to find modem!\r\n");

                    if( num_attempts < 100U )
                    {
                        num_attempts++;
                    }

                    osDelay( ( 5000U + ( 1000U * num_attempts ) ) );
                }
            }
            break;


#if MODEM_HAS_GPS
        case ST_ENABLE_GPS:
            check_conf__();

            PRINTF("ModemCtrl -- enabling GPS\r\n");
            enable_gps();
            osDelay(5000);
            s_state = ST_JOINING_NETWORK;
            break;
#endif


        case ST_JOINING_NETWORK:
            PRINTF("ModemCtrl -- Joining network\r\n");
            num_attempts = 0U;
            while( s_state == ST_JOINING_NETWORK )
            {
                check_conf__();

                if( registered_to_network() )
                {
                    PRINTF("ModemCtrl -- SUCCESS -- registered to network\r\n");
                    AlcLogger_log_info("Modem is registered to network");
                    s_state = ST_CONFIGURE_LINK;
                }
                else if( num_attempts++ < 200U )
                {
                    PRINTF("ModemCtrl -- ERROR -- Failed to register to network\r\n");
#if 1
                    osDelay(5000u);
#else
                    for(int ii=0; ii<=num_attempts; ii++)
                    {
                        osDelay(5000u);
                    }
#endif
                }
                else
                {
                    /* too many attempts -- try resetting the modem and starting again. */
                    AlcLogger_log_error("Modem could not register to network");
                    s_state = ST_INITIALISING;
                }
            }
            break;


        case ST_CONFIGURE_LINK:
            PRINTF("ModemCtrl -- configuring link\r\n");
            num_attempts = 0U;
            while( s_state == ST_CONFIGURE_LINK )
            {
                check_conf__();

#if 0
                /* don't enable mux here -- just go to running */
                s_state = ST_RUNNING;
#else
                if( configure_link() )
                {
                    PRINTF("ModemCtrl -- modem link configured\r\n");
                    AlcLogger_log_info("Modem running and connected to GPRS.");
                    s_state = ST_RUNNING;
                }
                else if( num_attempts++ < 25U )
                {
                    PRINTF("ModemCtrl -- failed to configure modem link!\r\n");
                    osDelay(2000u);
                }
                else
                {
                    /* too many attempts -- try resetting the modem and starting again. */
                    s_state = ST_INITIALISING;
                }
#endif
            }
            break;


        case ST_RUNNING:
            PRINTF("ModemCtrl -- running\r\n");
            num_attempts = 0U;
            while( s_state == ST_RUNNING )
            {
                check_conf__();

                if( is_connected__() )
                {
                    /* Modem connection is OK */
                    num_attempts = 0U;
                    osDelay(30000u);
                }
                else
                {
                    /* Modem connection is not OK */
                    if( num_attempts < 10U )
                    {
                        num_attempts++;
                        osDelay(2000u);
                    }
                    else
                    {
                        AlcLogger_log_critical("Modem CTRL detected modem is no longer connected");
                        s_state = ST_SHUTDOWN;
                    }
                }
            }
            break;


        case ST_SHUTDOWN:
            /*
             * Perform a controlled shutdown of the modem and then restart it
             */
            Modem_enable_gprs(false);
            AlcLogger_log_critical("Modem CTRL restarting modem in 30 seconds");
            osDelay(30000u);
            s_state = ST_INITIALISING;
            break;


        default:
            /* should not get here */
            AlcLogger_log_critical("Modem CTRL illegal state detected");
            s_state = ST_INITIALISING;
            osDelay(10u);
            break;
        }
    } /* for() */


    /* Should not get here -- reset the processor in case this is an error! */
    AlcLogger_log_critical("Modem CTRL thread is exiting -- rebooting in 4 seconds");
    osDelay(4000U);

    /* Do software reset */
    HAL_NVIC_SystemReset();
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static bool begin(void)
{
    s_gps_ready = false;

    PRINTF("Modem -- hard reset\r\n");
    Modem_hard_reset();
    osDelay(2000);

    uint8_t ch;

    PRINTF("Attempting to autobaud with modem using AT commands\r\n");

    // give 10 seconds to reboot
    int16_t timeout = 10000;

    while( timeout > 0 )
    {
        if( Modem_send_at() )
        {
            break;
        }

        osDelay(500);
        timeout -= 500;
    }

    if( timeout <= 0 )
    {
        PRINTF("Timeout: No response to AT... last ditch attempt.\r\n");
        Modem_send_at();
        osDelay(100);
        Modem_send_at();
        osDelay(100);
        Modem_send_at();
        osDelay(100);
    }

    osDelay(1000);

    return true;
}
/******************************************************************************/
#if MODEM_HAS_GPS
static void enable_gps(void)
{
    /* Enable GPS */
    Modem_run_command("AT+CGNSPWR=1", SEARCH_OK, 100);
    Modem_run_command("AT+CGNSSEQ=RMC", SEARCH_OK, 100);

    s_gps_ready = true;
}
#endif
/******************************************************************************/
static bool registered_to_network(void)
{
    bool success=false;

#if MODEM_IS_CELLULAR
    uint32_t status;

    Modem_enable_network_registration();

    if( Modem_get_network_registration(&status) )
    {
        if( status == MODEM_NW_REG_JOINED_HOME )
        {
            success = true;
        }
    }
#else
    // todo
#error "Non cellular case has not been written yet"
#endif

    return success;
}
/******************************************************************************/
static bool configure_link(void)
{
    Modem_enable_gprs(false);

    if( !Modem_enable_gprs(true) )
    {
        PRINTF("configure_link -- failed to enable GPRS\r\n");
        return false;
    }

    if( !Modem_get_ip_addr(NULL, 0) )
    {
        PRINTF("configure_link -- failed to get IP address\r\n");
        return false;
    }


#if SIM808_ENABLE_GPRS_FOR_NTP
    /* Set Modem to use NTP to initialise and keep it's internal RTC */
    if( !Modem_enable_ntp() )
    {
        PRINTF("configure_link -- failed to enable NTP\r\n");
        return false;
    }
#endif /* SIM808_ENABLE_GPRS_FOR_NTP */

    // todo

    return true;
}
/******************************************************************************/
static void check_conf__(void)
{
    if(s_restart_modem)
    {
        AlcLogger_log_info("Modem restart requested -- restarting modem");
        s_state = ST_SHUTDOWN;
    }


    if( ModemCtrl_check_conf_has_changed() )
    {
        AlcLogger_log_info("Modem configuration has changed -- restarting modem");
        s_state = ST_SHUTDOWN;
    }
}
/******************************************************************************/
static bool is_connected__(void)
{
    /**
     * To be connected to the network, the following must be true...
     *
     *  1. Must be registered to a cellular network.
     *  2. GPRS must be on
     */
    uint32_t network_status;
    uint32_t gprs_status;

    PRINTF("Modem CTRL testing connection...\r\n");

    if( Modem_get_network_registration(&network_status) )
    {
        PRINTF("Modem CTRL network_status = %u\r\n", network_status);

        if( network_status == MODEM_NW_REG_JOINED_HOME )
        {
            if( Modem_gprs_service_status(&gprs_status) )
            {
                PRINTF("Modem CTRL gprs_status    = %u\r\n", gprs_status);

                return  ( gprs_status != 0U );
            }
        }
    }


    PRINTF("Modem CTRL failed to read network and gprs status\r\n");


    return false;
}
/******************************************************************************/
