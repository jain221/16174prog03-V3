/**
 * @file  data_upload_client.c
 * @brief Thread to control uploading data to the cloud
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "data_upload_client.h"
#include "data_upload_client_conf.h"

#include "alc_eat_string_tokens.h"
#include "alc_ipaddr_snprintf.h"
#include "alc_logger.h"
#include "alc_string.h"
#include "cmsis_os.h"
#include "data_upload_msg.h"
#include "FreeRTOS.h"
#include "gps_time_ctrl.h"
#include "modem_ctrl.h"
#include "modem_drv.h"
#include "modem_drv_conf.h"
#include "nv_settings.h"
#include "sensor_data_pool.h"
#include "sensor_node_list.h"
#include "stm32xxxx_hal_cortex.h"
#include "sys/clock.h"


#define DEBUG_DONT_SEND_DATA_TO_CLOUD 0
#define DEBUG_STREAM 0
#define DEBUG DEBUG_NONE
#include "net-debug.h"
#include "uip-debug.h"


#if DEBUG
#include "uart3.h"
#else
#define UART3_write(...)                do { } while(0)
#endif




#ifndef MODEM_CHANNEL_DATA_UPLOAD_CLIENT
#error "MODEM_CHANNEL_DATA_UPLOAD_CLIENT has not been defined"
#endif




/*******************************************************************************
*                               LOCAL DEFINES
*******************************************************************************/

/** @def   DUC_NODE_LONG_MSG_INTERVAL_S
 *  @brief How often (in seconds) to send the long version of the node message to the cloud
 */
#ifndef DUC_NODE_LONG_MSG_INTERVAL_S
#error "DUC_NODE_LONG_MSG_INTERVAL_S has not been defined in data_upload_client_conf.h"
#endif


/** @def   DUC_RUN_CHECKS_INTERVAL_S
 *  @brief How often (in seconds) to send do housekeeping checks on nodes
 */
#ifndef DUC_RUN_CHECKS_INTERVAL_S
#error "DUC_RUN_CHECKS_INTERVAL_S has not been defined in data_upload_client_conf.h"
#endif


/** @def   DUC_LOST_CONN_INTERVAL_S
 *  @brief How long (in seconds) to wait without messages before deleting node.
 */
#ifndef DUC_LOST_CONN_INTERVAL_S
#error "DUC_LOST_CONN_INTERVAL_S has not been defined in data_upload_client_conf.h"
#endif


/** @def   DUC_LOOSING_CONN_INTERVAL_S
 *  @brief How long (in seconds) to wait without messages before flagging node as 'stale'
 */
#ifndef DUC_LOOSING_CONN_INTERVAL_S
#error "DUC_LOOSING_CONN_INTERVAL_S has not been defined in data_upload_client_conf.h"
#endif




/*******************************************************************************
*                               LOCAL CONSTANTS
*******************************************************************************/




/*******************************************************************************
*                               LOCAL DATA TYPES
*******************************************************************************/

typedef struct {
    uint8_t  ipv4_address[4];
    uint16_t portnum;
} IPConnection;




/*******************************************************************************
*                               LOCAL TABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL GLOBAL VARIABLES
*******************************************************************************/

extern osMessageQId g_data_upload_client_rx_queueHandle;

static char s_request_str[1500];
static bool s_need_retransmit_data=false;


static uint32_t s_last_gateway_tx_s=0U;
static uint32_t s_hourly_s=0U;


/** @brief A buffer to hold text lines received from the Cloud Server */
static struct {
    uint8_t  buff[40];      /**< @brief The buffer */
    uint16_t inp;           /**< @brief The next insertion point in the buffer */
} s_command_line;




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static bool modem_is_ready__(void);
static bool connect_to_server__(IPConnection const *p_connection);
static bool node_is_due_long_msg(SensorNode const *p_sensor_node);
static void mark_all_nodes_as_dirty__(void);
static bool mark_node_as_dirty__(uint32_t index, SensorNode *p_sensor_node);
static void poll_nodes__(void);
static bool process_node__(uint32_t index, SensorNode *p_sensor_node);
static void do_hourly_checks__(void);
static bool check_node_lost_comms__(uint32_t index, SensorNode *p_sensor_node);
static bool upload_buffer_to_cloud__(bool write_log);
static bool send_security_string_msg__(void);
static bool send_gateway_msg__(void);
static uint32_t tcp_link_send_size__(void);
static bool tcp_link_is_open__(void);
static bool print_ip_status_line__(char const *str);
static void dump_ip_status__(void);
static void command_line_reset__(void);
static void command_line_push_back__(uint8_t ch);
static void process_char_from_server__(char ch);
static void check_tim_reply__(char const *str);




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
/* todo maybe remove this event handler some day */
void HttpServer_connection_opened(void)
{
    PRINTF("HTTP Server -- connection opened\r\n");
}
/******************************************************************************/
/* todo maybe remove this event handler some day */
void HttpServer_connection_closed(void)
{
    PRINTF("HTTP Server -- connection closed\r\n");
}
/******************************************************************************/
void DataUploadClient_task(void const * argument)
{
    uint32_t count_open_failures=0U;
    IPConnection cloud_connection;

    osDelay(1000u);

    s_hourly_s = clock_seconds();
    command_line_reset__();

    s_need_retransmit_data = false;


#if DEBUG_DONT_SEND_DATA_TO_CLOUD
#if PRODUCTION_RELEASE
#error "Can't use fake time in PRODUCTION_RELEASE"
#endif
    /* Fake time */
    AlcLogger_log_warning("USING FAKE TIME FOR TESTING ONLY!!");
    have_timestamp_from_server(1511266219LU);
#endif /* DEBUG_DONT_SEND_DATA_TO_CLOUD */


    PRINTF("DataUploadClient -- Starting\r\n");

    /* main loop for task */
    for(;;)
    {
#if 0
        /* do nothing */
        osDelay(10000u);
#elif DEBUG_DONT_SEND_DATA_TO_CLOUD
        /* just delay -- so we don't hog the cpu */
        osDelay(1u);

        poll_nodes__();
#else
        /* run the data upload client */

        /* Test if Modem needs to be reset */
        if( count_open_failures >= DUC_RETRY_OPEN_LIMIT )
        {
            /* Have exceeded the limit...
             * reset the modem to try to clear the fault
             */
            count_open_failures=0U;
            ModemCtrl_restart_modem();
        }


        if( !modem_is_ready__() )
        {
            /* The modem is not ready yet */
            count_open_failures++;
        }
        else
        {
            /* The modem is ready...
             * Try to open a TCP link to the Cloud server and send data back and
             * forth.
             */
            PRINTF("################################################################\r\n");
            PRINTF("DataUploadClient -- opening TCP link...\r\n");

            /* Before we can open the TCP link, we need the IP address and
             * port number of the server (these are stored in non-volatile
             * memory).
             */
            if( !Store_read_cloud_ipv4(&cloud_connection.ipv4_address) )
            {
                /* The IP address is not set */
                AlcLogger_log_error("Data Upload Client failed to read cloud IP address from storage\r\n");
            }
            else if( !Store_read_cloud_portnum(&cloud_connection.portnum) )
            {
                /* The port number is not set */
                AlcLogger_log_error("Data Upload Client failed to read cloud port number from storage\r\n");
            }
            else if( connect_to_server__(&cloud_connection) )
            {
                /* We are now connected to the Cloud server
                 */
                count_open_failures=0U;

                PRINTF("DataUploadClient -- opened TCP link success\r\n");
                AlcLogger_log_info("Data Upload Client successfully opened TCP link to server");


                /**** resend string to the cloud server ****/
                if( s_need_retransmit_data )
                {
                    uint32_t len = strlen(s_request_str);

                    if( ( len > 0U ) && ( len < sizeof(s_request_str) ) )
                    {
                        /* Send buffer contents to cloud */
                        PRINTF("Sending %u bytes to cloud\r\n", len);
                        AlcLogger_log_info("Retransmitting data to server");
                        upload_buffer_to_cloud__(true);
                        s_need_retransmit_data = false;
                    }
                }


                /**** send security string to the cloud server ****/
                PRINTF("DataUploadClient -- sending security string\r\n");
                if( send_security_string_msg__() )
                {
                    PRINTF("DataUploadClient -- success, sent security string ok\r\n");
                }
                else
                {
                    PRINTF("DataUploadClient -- error, failed to send security string!\r\n");
                }


                /**** send info on gateway ****/
                PRINTF("DataUploadClient -- sending gateway info\r\n");
                if( send_gateway_msg__() )
                {
                    PRINTF("DataUploadClient -- success, sent gateway info ok\r\n");
                }
                else
                {
                    PRINTF("DataUploadClient -- error, failed to send gateway info!\r\n");
                }


                mark_all_nodes_as_dirty__();


                s_last_gateway_tx_s = clock_seconds();


                command_line_reset__();

                /*
                 * Loop here as long as the TCP/IP link is open
                 */
                while( tcp_link_is_open__() )
                {
                    /* read all data from the rx queue */
                    char ch;
                    while( xQueueReceive(g_data_upload_client_rx_queueHandle, &ch, 1) == pdTRUE)
                    {
                        /* Got a character from queue */
                        process_char_from_server__(ch);
                    }

                    poll_nodes__();
                }


                /* Make sure the TCP link is closed */
                PRINTF("DataUploadClient -- closing TCP link...\r\n");
                if( Modem_tcp_close(MODEM_CHANNEL_DATA_UPLOAD_CLIENT, 10000) )
                {
                    PRINTF("DataUploadClient -- closed TCP link ok\r\n");
                }
                else
                {
                    PRINTF("DataUploadClient -- closed TCP link failed!\r\n");
                }

                AlcLogger_log_warning("Data Upload Client detected TCP link to server has been closed");
            }
            else
            {
                /* We failed to connect to the Cloud server
                 */
                count_open_failures++;
                PRINTF("DataUploadClient -- error - failed to open TCP link!\r\n");
            }
        }

        PRINTF("DataUploadClient -- will try to open the TCP link in 30 seconds\r\n");
        osDelay(DUC_RETRY_OPEN_PERIOD_MS);
#endif
    } /* for() */


    /* Should not get here -- reset the processor in case this is an error! */
    AlcLogger_log_critical("Data Upload Client thread is exiting -- rebooting in 4 seconds");
    osDelay(4000U);

    /* Do software reset */
    HAL_NVIC_SystemReset();
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static bool modem_is_ready__(void)
{
#if 1
    return ModemCtrl_modem_is_ready();
#else
    uint32_t network_status;
    uint32_t gprs_status;
    bool is_ready=false;

    if( !ModemCtrl_modem_is_ready() )
    {
        // modem controller is not ready
    }
    else if( !Modem_get_network_registration(&network_status) )
    {
        /* failed to get modem's network registration status */
    }
    else if( network_status != MODEM_NW_REG_JOINED_HOME )
    {
        /* network status is not 1 */
        PRINTF("network status is not 1\r\n");
    }
    else if( !Modem_gprs_service_status(&gprs_status) )
    {
        /* failed to get modem's GPRS status */
    }
    else if( gprs_status != 1U )
    {
        /* GPRS status is not 1 */
        PRINTF("GPRS status is not 1 (%u)\r\n", gprs_status);
    }
    else
    {
        /* all ok -- modem is ready to tx data */
        is_ready = true;
    }

    return is_ready;
#endif
}
/******************************************************************************/
static bool connect_to_server__(IPConnection const *p_connection)
{
    bool success = false;

    if( tcp_link_is_open__() )
    {
        /* Trying to connect to server but Modem is reporting that the link is
         * open...
         * Just close it here
         * Assume our state has got out of sync with the modem's state
         */
        AlcLogger_log_warning("Data Upload Client forcing TCP closed");
        if( Modem_tcp_close(MODEM_CHANNEL_DATA_UPLOAD_CLIENT, 10000) )
        {
            PRINTF("Data Upload Client -- closed TCP link ok\r\n");
        }
        else
        {
            PRINTF("Data Upload Client -- closed TCP link failed!\r\n");
        }
    }

    if(p_connection)
    {
        uint8_t server[18];

        /* get IP address as a string */
        snprintf(server, sizeof(server),
                "%u.%u.%u.%u",
                p_connection->ipv4_address[0],
                p_connection->ipv4_address[1],
                p_connection->ipv4_address[2],
                p_connection->ipv4_address[3]);

        PRINTF("Data Upload Client opening TCP link to '%s:%u'", server, p_connection->portnum);
        success = Modem_tcp_open(MODEM_CHANNEL_DATA_UPLOAD_CLIENT, server, p_connection->portnum, 10000);

        if(success)
        {
            AlcLogger_log_printf(ALC_LOGGER_INFO, "Data Upload Client successfully opened TCP link to '%s:%u'", server, p_connection->portnum);
        }
        else
        {
            AlcLogger_log_printf(ALC_LOGGER_ERROR, "Data Upload Client failed to open TCP link to '%s:%u'", server, p_connection->portnum);
        }
    }

    return success;
}
/******************************************************************************/
static bool node_is_due_long_msg(SensorNode const *p_sensor_node)
{
    if(p_sensor_node)
    {
        if(
                ( p_sensor_node->last_long_msg_s == 0U ) ||
                ( ( clock_seconds() - p_sensor_node->last_long_msg_s ) >= DUC_NODE_LONG_MSG_INTERVAL_S )
        )
        {
            return true;
        }
    }

    return false;
}
/******************************************************************************/
static void mark_all_nodes_as_dirty__(void)
{
    SNL_for_each_node(&mark_node_as_dirty__);
}
/******************************************************************************/
static bool mark_node_as_dirty__(uint32_t index, SensorNode *p_sensor_node)
{
    SensorNode_mark_as_dirty(p_sensor_node);

    return true;
}
/******************************************************************************/
static void poll_nodes__(void)
{
    /* Process each node */
    SNL_for_each_node(&process_node__);


    /* Send gateway info every 10 minutes */
    if( ( clock_seconds() - s_last_gateway_tx_s ) >= (DUC_GATEWAY_MSG_INTERVAL_S) )
    {
        send_gateway_msg__();
        s_last_gateway_tx_s = clock_seconds();
    }


    /* Check every hour */
    if( ( clock_seconds() - s_hourly_s ) >= (DUC_RUN_CHECKS_INTERVAL_S) )
    {
        do_hourly_checks__();
        s_hourly_s = clock_seconds();
    }
}
/******************************************************************************/
static bool process_node__(uint32_t index, SensorNode *p_sensor_node)
{
    bool error_free=true;

    if( p_sensor_node )
    {
        uint32_t datalen = SensorNode_get_data_size(p_sensor_node);

        if( tcp_link_send_size__() == 0U )
        {
            /* Detected TCP link is closed...
             * can't send any data
             */
            PRINTF("Detected TCP link is closed!\r\n");
            error_free = false;
        }
        else
        {
            /* TCP link is open...
             */
            bool sending_node_message=false;


            /* Initialise the buffer. We will write data to this buffer before
             * sending the buffer contents to the Modem for transmission in one
             * go.
             */
            s_request_str[0] = '\0';


            /* Test if we should send a Node message */
            if(
                    ( SensorNode_is_dirty(p_sensor_node) ) ||
                    ( node_is_due_long_msg(p_sensor_node) )
            )
            {
                /* Send long Node message...
                 * There may be data to follow this message
                 */
                SensorNode_clear_is_dirty(p_sensor_node);
                sending_node_message = true;
                prepare_node_long_msg(s_request_str, sizeof(s_request_str), p_sensor_node);
                p_sensor_node->last_long_msg_s = clock_seconds();
            }
            else if( datalen > 0U )
            {
                /* Send short Node message...
                 * There will be data to follow this message
                 */
                sending_node_message = true;
                prepare_node_msg(s_request_str, sizeof(s_request_str), p_sensor_node);
            }
            else
            {
                /* don't send any Node message */
            }


            if(sending_node_message)
            {
                /* We are sending a Node message...
                 * If there is data for the Node, then send the data also...
                 */
                if( datalen > 0U )
                {
                    /* Send data for the Node */
#if DEBUG_STREAM
                    PRINTF("uploading data to cloud\r\n");
#endif

                    // flush data
                    uint32_t loopcount=0U;
                    for(
                            struct SensorData *p_sensor_data = SensorNode_remove_data(p_sensor_node);
                            p_sensor_data != NULL;
                            p_sensor_data = SensorNode_remove_data(p_sensor_node)
                    )
                    {
#if DEBUG_STREAM
                        PRINTF("  uploading data = %lu.%02u: %lu\r\n", p_sensor_data->ts_seconds, p_sensor_data->ts_hundreths, p_sensor_data->seq32);
#endif

                        uint32_t len = strlen(s_request_str);

                        /* add Data message to buffer */
                        prepare_data_msg(&s_request_str[len], sizeof(s_request_str) - len, p_sensor_data);

                        /* return data object to the empty pool */
                        SensorDataPool_return(p_sensor_data);

                        loopcount++;
                        if( loopcount > 20U )
                        {
                            break;
                        }
                    }
                }


                /* Send buffer contents to cloud */
                PRINTF("Sending %u bytes to cloud\r\n", strlen(s_request_str));
                error_free = upload_buffer_to_cloud__(true);

                if( (!error_free) && ( datalen > 0U ) )
                {
                    /* Failed to upload data to the Cloud...
                     * Set flag here so data buffer will be retransmitted the
                     * next time the Modem link is opened.
                     */
                    s_need_retransmit_data = true;
                }
            }
        }
    }

    return error_free;
}
/******************************************************************************/
static void do_hourly_checks__(void)
{
    PRINTF("Doing hourly checks\r\n");

    /* Process each node */
    SNL_for_each_node(&check_node_lost_comms__);

    uint32_t deleted_count = SNL_remove_deleted_nodes();
    if( deleted_count > 0U )
    {
        AlcLogger_log_printf(ALC_LOGGER_INFO, "Deleted %u nodes", deleted_count);
    }
}
/******************************************************************************/
static bool check_node_lost_comms__(uint32_t index, SensorNode *p_sensor_node)
{
    if( p_sensor_node )
    {
        PRINTF("Checking ");
        PRINT6ADDR(&p_sensor_node->ipaddr);
        PRINTF("\r\n");

        uint32_t seconds_since_last_msg_rx = SensorNode_seconds_since_last_msg_rx(p_sensor_node);
        bool send_node_status_message=false;

        if( seconds_since_last_msg_rx >= DUC_LOST_CONN_INTERVAL_S )
        {
            /****************************************************************
             * Time since last message from the node exceeds Lost-Connection
             * Delete the node and send a message to the Cloud server
             ***************************************************************/
            char ip_str[28];
            alc_ipaddr_snprintf(ip_str, sizeof(ip_str), &p_sensor_node->ipaddr);
            AlcLogger_log_printf(ALC_LOGGER_INFO, "Node %s marked for deletion", ip_str);

            SensorNode_mark_for_deletion(p_sensor_node);

            send_node_status_message = true;
        }
        else if( seconds_since_last_msg_rx >= DUC_LOOSING_CONN_INTERVAL_S )
        {
            /****************************************************************
             * Time since last message from the node exceeds Loosing-Connection
             * Flag the node as stale and send a message to the Cloud server
             ***************************************************************/
            PRINTF("  ");
            PRINT6ADDR(&p_sensor_node->ipaddr);
            PRINTF(" >> loosing comms (%u s)\r\n", SensorNode_seconds_since_last_msg_rx(p_sensor_node));

            SensorNode_mark_as_stale(p_sensor_node);

            send_node_status_message = true;
        }
        else
        {
            /* do nothing here */
        }


        if(send_node_status_message)
        {
            prepare_node_long_msg(s_request_str, sizeof(s_request_str), p_sensor_node);
            p_sensor_node->last_long_msg_s = clock_seconds();

            upload_buffer_to_cloud__(true);
        }
    }
}
/******************************************************************************/
static bool upload_buffer_to_cloud__(bool write_log)
{
#if DEBUG_DONT_SEND_DATA_TO_CLOUD
    /* Just print data to the screen */
    bool success = true;
    PRINTF(s_request_str);
#else
    /* send data to the Modem */
    bool success = Modem_tcp_write_str(MODEM_CHANNEL_DATA_UPLOAD_CLIENT, s_request_str, 4000);
#endif

    if(!success)
    {
        PRINTF("Data Upload Client -- error sending data to cloud!\r\n");

        if(write_log)
        {
            AlcLogger_log_error("Data Upload Client failed to send data to cloud");
        }
    }

    return success;
}
/******************************************************************************/
static bool send_security_string_msg__(void)
{
    prepare_security_string_msg(s_request_str, sizeof(s_request_str));

    return upload_buffer_to_cloud__(true);
}
/******************************************************************************/
static bool send_gateway_msg__(void)
{
    prepare_gateway_msg(s_request_str, sizeof(s_request_str));

    return upload_buffer_to_cloud__(true);
}
/******************************************************************************/
static uint32_t tcp_link_send_size__(void)
{
    uint32_t send_size=0U;

    (void) Modem_tcp_get_send_size(MODEM_CHANNEL_DATA_UPLOAD_CLIENT, &send_size, 2000U);

    return send_size;
}
/******************************************************************************/
static bool tcp_link_is_open__(void)
{
    return ( tcp_link_send_size__() > 0U );
}
/******************************************************************************/
static bool print_ip_status_line__(char const *str)
{
    if(str)
    {
        PRINTF("  IP Status >>>>%s<<<<\r\n", str);
    }

    return true;
}
/******************************************************************************/
static void dump_ip_status__(void)
{
    PRINTF("dump IP status\r\n");

    osDelay(500u);
    Modem_run_command_ex("AT+CIPSTATUS",
                         SEARCH_ERROR,
                         &print_ip_status_line__,
                         NULL,
                         0U,
                         5000U);

    osDelay(500u);
}
/******************************************************************************/
static void command_line_reset__(void)
{
    s_command_line.buff[0] = '\0';
    s_command_line.inp     = 0U;
}
/******************************************************************************/
static void command_line_push_back__(uint8_t ch)
{
    if( s_command_line.inp < ( sizeof(s_command_line.buff) - 2 ) )
    {
        s_command_line.buff[s_command_line.inp] = ch;
        s_command_line.inp++;
        s_command_line.buff[s_command_line.inp] = '\0';
    }
}
/******************************************************************************/
static void process_char_from_server__(char ch)
{
    if( (ch=='\r') || (ch=='\n') )
    {
        if( strlen(s_command_line.buff) > 0 )
        {
            check_tim_reply__(s_command_line.buff);
        }

        command_line_reset__();
    }
    else
    {
        /* append character to line */
        command_line_push_back__(ch);
    }
}
/******************************************************************************/
static void check_tim_reply__(char const *str)
{
    /** Expect the line to be 'tim,timestamp'
     */
    if( strncmp(str, "tim,", 4) == 0 )
    {
        uint32_t timestamp;
        bool success;

        str = &str[4];

        success =  eat_u32(&str, &timestamp);   /* extract timestamp */

        if( success )
        {
            PRINTF("got timestamp %u\r\n", timestamp);
            have_timestamp_from_server(timestamp);
        }
    }
}
/******************************************************************************/
