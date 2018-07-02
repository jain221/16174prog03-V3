/**
 * @file  modem_drv_sim808.c
 * @brief Modem driver (for SIM808 GSM modem module)
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "modem_drv.h"
#include "modem_drv_conf.h"

#include "alc_eat_string_tokens.h"
#include "alc_store_string_utils.h"
#include "alc_string.h"
#include "alc_test_char_seq.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include "http_server.h"
#include "task.h"
#include "uart6.h"


#define DEBUG DEBUG_NONE
#include "net-debug.h"


#if DEBUG
#include "uart3.h"

#define DEBUG_PRINTLN(replybuffer)      do { PRINTF("<<<<%s>>>>\r\n", replybuffer); } while(0)
#else
#define UART3_write(...)                do { } while(0)
#endif




#ifndef prog_char_strcmp
#define prog_char_strcmp(a, b)                  strcmp((a), (b))
#endif

#ifndef prog_char_strstr
#define prog_char_strstr(a, b)                  strstr((a), (b))
#endif

#ifndef prog_char_strlen
#define prog_char_strlen(a)                     strlen((a))
#endif


#ifndef prog_char_strcpy
#define prog_char_strcpy(to, fromprogmem)       strcpy((to), (fromprogmem))
#endif




/*******************************************************************************
*                               LOCAL DEFINES
*******************************************************************************/

#define SIM808_NUM_CHANNELS         6U
#define SIM808_ENABLE_GPRS_FOR_NTP  0




/*******************************************************************************
*                               LOCAL CONSTANTS
*******************************************************************************/

static const uint8_t ASCII_LF = 0x0A;   /* LINE FEED           */
static const uint8_t ASCII_CR = 0x0D;   /* CARRAGE RETURN      */




/*******************************************************************************
*                               LOCAL DATA TYPES
*******************************************************************************/

typedef enum
{
    RXST_IDLE = 0,
    RXST_COMMAND,
    RXST_GT,
    RXST_TX_DATA,
    RXST_TX_DATA_COMPLETE,
    RXST_IPD_PORTNUM,
    RXST_IPD_NUMBYTES
} RxState;




/*******************************************************************************
*                               LOCAL TABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL GLOBAL VARIABLES
*******************************************************************************/

static struct {
    RxState   rx_state;

    struct {
        char const *command_str;
        uint32_t    search_mask;
        bool (*fn)(char const *str);
        bool        result;
        bool        active;


        struct {
            uint32_t tx_bufflen;
            uint32_t tx_count;
            bool ready_to_send;
        } tx_data;
    } current_command;


    volatile uint32_t num_errors;

    volatile bool tcp_link_is_open;

    volatile bool          command_result;
} s_task_data;


uint32_t ipd_portnum_;
uint32_t ipd_numbytes_;


static AlcTestCharSeq s_current_command;
static AlcTestCharSeq s_ipd;
static AlcTestCharSeq s_open;
static AlcTestCharSeq s_closed;


/** @brief The handle for the TCP receive queue belonging to the data-upload-client */
extern osMessageQId g_data_upload_client_rx_queueHandle;


/* @brief The mutex object
 * @note This object is created in freertos.c
 */
extern osMutexId g_modem_drv_mutexHandle;




/** @brief Var used by Modem_tcp_get_send_size() function
 */
static volatile struct {
    uint32_t size;
    uint8_t  channel;
    bool     success;
} s_tcp_get_send_size;




/** @brief Var used by Modem_get_rtc() function
 */
static volatile struct {
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;
    bool     success;
} s_get_rtc_data;




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static bool check_tcp_get_send_size_reply__(char const *str);
static bool check_get_rtc_reply__(char const *str);
static bool run_command_ex__(char const *command_str, uint32_t search_mask, bool (*fn)(char const *str), uint8_t const* p_tx_buff, uint32_t tx_bufflen, uint32_t timeout_ms);
static bool start_command(char const *command_str, uint32_t search_mask, bool (*fn)(char const *str), uint32_t tx_bufflen);
static void process_rx_char__(uint8_t ch);




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/




static uint8_t  replybuffer[255];
static uint16_t replybuffer_inp=0U;

/******************************************************************************/
static inline void ReplyBuffer_reset(void)
{
    replybuffer[0]  = '\0';
    replybuffer_inp = 0U;
}
/******************************************************************************/
static inline void ReplyBuffer_push_back(uint8_t ch)
{
    if( replybuffer_inp < ( sizeof(replybuffer) - 2 ) )
    {
        replybuffer[replybuffer_inp] = ch;
        replybuffer_inp++;
        replybuffer[replybuffer_inp] = '\0';
    }
}
/******************************************************************************/




/******************************************************************************/
static inline bool acquire_mutex__(uint32_t timeout_ms)
{
    return ( osMutexWait(g_modem_drv_mutexHandle, timeout_ms) == osOK );
}
/******************************************************************************/
static inline void release_mutex__(void)
{
    osMutexRelease(g_modem_drv_mutexHandle);
}
/******************************************************************************/




/******************************************************************************/
bool Modem_run_command(char const *command_str, uint32_t search_mask, uint32_t timeout_ms)
{
    return Modem_run_command_ex(command_str, search_mask, NULL, NULL, 0U, timeout_ms);
}
/******************************************************************************/
bool Modem_run_command_tx_data(char const *command_str, uint32_t search_mask, uint8_t const* p_tx_buff, uint32_t tx_bufflen, uint32_t timeout_ms)
{
    return Modem_run_command_ex(command_str, search_mask, NULL, p_tx_buff, tx_bufflen, timeout_ms);
}
/******************************************************************************/




static struct {
    char const *p_search_str;
    uint32_t    search_str_len;
    int32_t     num;
    uint32_t    index;
    bool        success;
} volatile s_run_command_parse_reply={0U};


static bool check_run_command_parse_reply(char const *str)
{
    if(
            ( s_run_command_parse_reply.p_search_str ) &&
            ( s_run_command_parse_reply.search_str_len > 0U ) &&
            ( strncmp(str, s_run_command_parse_reply.p_search_str, s_run_command_parse_reply.search_str_len) == 0 )
    )
    {
        str=&str[s_run_command_parse_reply.search_str_len];

        PRINTF("STR = %s\r\n", str);

        s_run_command_parse_reply.num     = 0;
        s_run_command_parse_reply.success = false;

        uint32_t temp_u32;

        for(uint32_t ii=0U; ii<=s_run_command_parse_reply.index; ii++)
        {
            if( !eat_u32(&str, &temp_u32) )
            {
                break;
            }
            else if(ii == s_run_command_parse_reply.index )
            {
                // success -- got it
                s_run_command_parse_reply.num     = temp_u32;
                s_run_command_parse_reply.success = true;
                break;
            }
            else if( !eat_char(&str, ',') )
            {
                // failed
            }
            else
            {
                // carry on with next number
            }
        }
    }

    return true;
}




bool Modem_run_command_parse_reply_u32(
        char const *command_str,
        char const *p_search_str,
        int32_t    *p_num,
        uint32_t    index,
        uint32_t search_mask,
        uint32_t timeout_ms)
{
    bool success=false;

    if(
            (command_str) &&
            (p_search_str)
    )
    {
        if( acquire_mutex__(timeout_ms) )
        {
            s_run_command_parse_reply.p_search_str = p_search_str;
            s_run_command_parse_reply.search_str_len = strlen(p_search_str);
            s_run_command_parse_reply.index = index;
            s_run_command_parse_reply.num = 0;

            if( run_command_ex__(command_str, search_mask, &check_run_command_parse_reply, NULL, 0U, timeout_ms) )
            {
                success = s_run_command_parse_reply.success;
            }

            if(p_num)
            {
                *p_num = s_run_command_parse_reply.num;
            }

            release_mutex__();
        }
    }

    return success;
}
/******************************************************************************/
bool Modem_run_command_ex(char const *command_str, uint32_t search_mask, bool (*fn)(char const *str), uint8_t const* p_tx_buff, uint32_t tx_bufflen, uint32_t timeout_ms)
{
    bool succes=false;

    if( acquire_mutex__(timeout_ms) )
    {
        succes = run_command_ex__(command_str, search_mask, fn, p_tx_buff, tx_bufflen, timeout_ms);

        release_mutex__();
    }

    return succes;
}
/******************************************************************************/
void Modem_hard_reset(void)
{
    HAL_GPIO_WritePin(MODEM_RST_GPIO_Port, MODEM_RST_Pin, GPIO_PIN_RESET);
    osDelay(500);
    HAL_GPIO_WritePin(MODEM_RST_GPIO_Port, MODEM_RST_Pin, GPIO_PIN_SET);
}
/******************************************************************************/
bool Modem_soft_reset(void)
{
    return Modem_run_command("AT+RST", SEARCH_OK, 5000);
}
/******************************************************************************/
bool Modem_send_at(void)
{
    return Modem_run_command("AT", SEARCH_OK, 100);
}
/******************************************************************************/
bool Modem_status(void)
{
    return Modem_run_command("AT+CIPSTATUS", SEARCH_OK, 10000);
}
/******************************************************************************/
bool Modem_enable_network_registration(void)
{
    return Modem_run_command("AT+CREG=1", SEARCH_OK|SEARCH_ERROR, 10000);
}
/******************************************************************************/
bool Modem_get_network_registration(uint32_t *p_status)
{
    if(p_status)
    {
        *p_status=0U;
    }

    return Modem_run_command_parse_reply_u32(
            "AT+CREG?",
            "+CREG: ",
            p_status,
            1,
            SEARCH_OK|SEARCH_ERROR,
            1000);
}
/******************************************************************************/
bool Modem_gprs_service_status(uint32_t *p_status)
{
    if(p_status)
    {
        *p_status=0U;
    }

    return Modem_run_command_parse_reply_u32(
            "AT+CGATT?",
            "+CGATT: ",
            p_status,
            0,
            SEARCH_OK|SEARCH_ERROR,
            1000);
}
/******************************************************************************/
bool Modem_get_sim_ccid(void)
{
    return Modem_run_command("AT+CCID", SEARCH_OK|SEARCH_ERROR, 10000);
}
/******************************************************************************/
bool Modem_is_password_required(void)
{
    return Modem_run_command("AT+CPIN?", SEARCH_OK|SEARCH_ERROR, 10000);
}
/******************************************************************************/
bool Modem_unlock_sim(char const *p_pin)
{
    bool success=false;

    if( (p_pin) && ( strlen(p_pin) == 4 ) )
    {
        char send_buff[14];

        snprintf(send_buff, sizeof(send_buff), "AT+CPIN=%s", p_pin);

        success = Modem_run_command(send_buff, SEARCH_OK|SEARCH_ERROR, 10000);
    }

    return success;
}
/******************************************************************************/
bool Modem_enable_gprs(bool on_off)
{
    if(on_off)
    {
        uint8_t cmd[40];

        /* disconnect all sockets */
        PRINTF("disconnect all sockets\r\n");
        Modem_run_command("AT+CIPSHUT", SEARCH_SHUT_OK|SEARCH_ERROR, 20000U);


        PRINTF("send AT+CGATT=1\r\n");
        if( !Modem_run_command("AT+CGATT=1", SEARCH_OK|SEARCH_ERROR, 10000U) )
        {
            PRINTF("Failed to set GATT=1\r\n");
            return false;
        }


#if 1
        PRINTF("send AT+CIPMUX=1\r\n");
        if( !Modem_run_command("AT+CIPMUX=1", SEARCH_OK|SEARCH_ERROR, 10000U) )
        {
            return false;
        }
#endif


#if SIM808_ENABLE_GPRS_FOR_NTP
        // set bearer profile! connection type GPRS
        PRINTF("set bearer profile! connection type GPRS\r\n");
        if( !Modem_run_command("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",
                             SEARCH_OK|SEARCH_ERROR,
                             10000U) )
        {
            PRINTF(">>>>>>>>>>> dwr failed AT+SAPBR=\r\n");
            return false;
        }


        // set bearer profile access point name
        build_at_sapbr_apn_cmd(cmd, sizeof(cmd), 1U);
        PRINTF("send command %s\r\n", cmd);
        if( !Modem_run_command(cmd, SEARCH_OK|SEARCH_ERROR, 5000U) )
        {
            return false;
        }
#endif /* SIM808_ENABLE_GPRS_FOR_NTP */


        // Set APN name, user, password
        PRINTF("Set APN name, user, password\r\n");
        build_at_cstt_cmd(cmd, sizeof(cmd));
        if( !Modem_run_command(cmd, SEARCH_OK|SEARCH_ERROR, 20000U) )
        {
            return false;
        }
        osDelay(10000);


#if SIM808_ENABLE_GPRS_FOR_NTP
        // open GPRS context
        PRINTF("open GPRS context\r\n");
        if( !Modem_run_command("AT+SAPBR=1,1",
                             SEARCH_OK|SEARCH_ERROR,
                             30000U) )
        {
            return false;
        }
#endif /* SIM808_ENABLE_GPRS_FOR_NTP */

        // bring up wireless connection
        PRINTF("bring up wireless connection\r\n");
        if( !Modem_run_command("AT+CIICR", SEARCH_OK|SEARCH_ERROR, 5000U) )
        {
            return false;
        }
    }
    else
    {
        // disconnect all sockets
        if( !Modem_run_command("AT+CIPSHUT", SEARCH_SHUT_OK|SEARCH_ERROR, 20000U) )
        {
            return false;
        }

#if SIM808_ENABLE_GPRS_FOR_NTP
        // close GPRS context
        if( !Modem_run_command("AT+SAPBR=0,1", SEARCH_OK|SEARCH_ERROR, 10000U) )
        {
            return false;
        }
#endif /* SIM808_ENABLE_GPRS_FOR_NTP */

        if( !Modem_run_command("AT+CGATT=0", SEARCH_OK|SEARCH_ERROR, 10000U) )
        {
            return false;
        }
    }

    return true;
}
/******************************************************************************/
#if SIM808_ENABLE_GPRS_FOR_NTP
bool Modem_enable_ntp(void)
{
    PRINTF("Enabling NTP to keep Modem RTC clock in sync\r\n");

    /* Set NTP to use bear profile 1 */
    Modem_run_command("AT+CNTPCID=1", SEARCH_OK|SEARCH_ERROR, 10000U);

    /* Set NTP url */
#if 1
    Modem_run_command("AT+CNTP=\"uk.pool.ntp.org\",0", SEARCH_OK|SEARCH_ERROR, 10000U);
#else
    Modem_run_command("AT+CNTP=\"202.120.2.101\",0", SEARCH_OK|SEARCH_ERROR, 10000U);
#endif

    /* Start Sync Network Time */
    Modem_run_command("AT+CNTP", SEARCH_ERROR, 2000U);

    return true;
}
#endif /* SIM808_ENABLE_GPRS_FOR_NTP */
/******************************************************************************/
double Modem_get_signal_strength(void)
{
    uint32_t rssi;

    if( Modem_run_command_parse_reply_u32(
            "AT+CSQ",
            "+CSQ: ",
            &rssi,
            0,
            SEARCH_OK|SEARCH_ERROR,
            1000) )
    {
        double sig_strength = (double) rssi;

        sig_strength *= 2.0f;
        sig_strength -= 115.0f;

        return sig_strength;
    }

    return 0.0f;
}
/******************************************************************************/
static struct {
    volatile char ip_address[25];
} s_get_ip_addr_data;
/******************************************************************************/
static bool check_ip_address__(char const *str)
{
    if( str )
    {
        if( strlen(str) > 0 )
        {
            strncpy_safe(s_get_ip_addr_data.ip_address, str, sizeof(s_get_ip_addr_data.ip_address));

            s_task_data.current_command.result = true;
            s_task_data.current_command.active = false;
            s_task_data.rx_state = RXST_IDLE;

            return false;
        }
    }

    return true;
}
/******************************************************************************/
bool Modem_get_ip_addr(char *dest, size_t destlen)
{
    bool success=false;

    if( acquire_mutex__(1000) )
    {
        s_get_ip_addr_data.ip_address[0] = '\0';

        if( run_command_ex__("AT+CIFSR", 0, &check_ip_address__, NULL, 0U, 100U) )
        {
            PRINTF("ip_address >>>>%s<<<<\r\n", s_get_ip_addr_data.ip_address);

            strncpy_safe(dest, s_get_ip_addr_data.ip_address, destlen);

            success = true;
        }

        release_mutex__();
    }

    return success;
}
/******************************************************************************/
bool Modem_enable_mux(bool enable)
{
    char str[15];

    snprintf(str, sizeof(str), "AT+CIPMUX=%u", ( enable ? 1u : 0u ) );

    return Modem_run_command(str, SEARCH_OK | SEARCH_ERROR, 1000);
}
/******************************************************************************/
bool Modem_tcp_write_buff(uint8_t channel, char const *p_buff, uint32_t bufflen, uint32_t timeout_ms)
{
    bool success=false;

    if( ( channel < SIM808_NUM_CHANNELS ) && (p_buff) && (bufflen>0U) )
    {
        char str[25];

        PRINTF("Modem_tcp_write_buff(%u, %u) \r\n", channel, bufflen);

        snprintf(str, sizeof(str), "AT+CIPSEND=%u,%u", channel, bufflen );

        success = Modem_run_command_tx_data(str, SEARCH_BUSY_P, p_buff, bufflen, timeout_ms);
    }

    return success;
}
/******************************************************************************/
bool Modem_tcp_write_str(uint8_t channel, char const *p_str, uint32_t timeout_ms)
{
    if( p_str )
    {
        return Modem_tcp_write_buff(channel, p_str, strlen(p_str), timeout_ms);
    }

    return false;
}
/******************************************************************************/
static struct {
    char  search_for_line[20];
    bool  success;
} volatile s_tcp_open_close_cmd={0U};
/******************************************************************************/
static bool check_tcp_open_close_reply(char const *str)
{
    if(str)
    {
        if( strcmp(str, s_tcp_open_close_cmd.search_for_line) == 0 )
        {
            /* found the line -- signal success */
            s_task_data.current_command.result = true;
            s_task_data.current_command.active = false;
            s_task_data.rx_state = RXST_IDLE;
        }
    }
    return true;
}
/******************************************************************************/
bool Modem_tcp_open(uint8_t channel, char const *server, uint16_t port, uint32_t timeout_ms)
{
    bool success=false;

    if(
            (server) &&
            (channel < SIM808_NUM_CHANNELS)
    )
    {
        if( acquire_mutex__(timeout_ms) )
        {
            char command[100];

            PRINTF("Modem_tcp_open(%u, %s, %u) \r\n", channel, server, port);

            snprintf(command, sizeof(command), "AT+CIPSTART=%u,\"TCP\",\"%s\",\"%u\"", channel, server, port);
            snprintf(s_tcp_open_close_cmd.search_for_line, sizeof(s_tcp_open_close_cmd.search_for_line), "%u, CONNECT OK", channel);

            if( run_command_ex__(command, SEARCH_ERROR, &check_tcp_open_close_reply, NULL, 0U, timeout_ms) )
            {
                success = s_run_command_parse_reply.success;
            }

            release_mutex__();
        }
    }

    return success;
}
/******************************************************************************/
bool Modem_tcp_close(uint8_t channel, uint32_t timeout_ms)
{
    bool success=false;

    if(channel < SIM808_NUM_CHANNELS)
    {
        if( acquire_mutex__(timeout_ms) )
        {
            char command[20];

            PRINTF("Modem_tcp_close(%u) \r\n", channel);

            snprintf(command, sizeof(command), "AT+CIPCLOSE=%u", channel );
            snprintf(s_tcp_open_close_cmd.search_for_line, sizeof(s_tcp_open_close_cmd.search_for_line), "%u, CLOSE OK", channel);

            if( run_command_ex__(command, SEARCH_ERROR, &check_tcp_open_close_reply, NULL, 0U, timeout_ms) )
            {
                success = s_run_command_parse_reply.success;
            }

            release_mutex__();
        }
    }

    return success;
}
/******************************************************************************/
static bool check_tcp_get_send_size_reply__(char const *str)
{
    /** Expect the line to be '+CIPSEND: channel,size'
     */
    if(
            ( strncmp(str, "+CIPSEND: ", 10) == 0 )
    )
    {
        uint32_t channel;
        uint32_t size;
        bool success;

        str = &str[10];


        success =  eat_u32(&str, &channel); /* extract channel number */
        success &= eat_comma(&str);
        success &= eat_u32(&str, &size);    /* extract size */

        if(
                ( success ) &&
                ( channel == s_tcp_get_send_size.channel )
        )
        {
            s_tcp_get_send_size.size    = size;
            s_tcp_get_send_size.success = true;
        }
    }

    return true;
}
/******************************************************************************/
bool Modem_tcp_get_send_size(uint8_t channel, uint32_t *p_size, uint32_t timeout_ms)
{
    bool success=false;

    if(p_size)
    {
        if( acquire_mutex__(1000U) )
        {
            s_tcp_get_send_size.size    = 0U;
            s_tcp_get_send_size.channel = channel;
            s_tcp_get_send_size.success = false;

            run_command_ex__("AT+CIPSEND?", SEARCH_OK|SEARCH_ERROR, &check_tcp_get_send_size_reply__, NULL, 0U, 1000U);

            *p_size = s_tcp_get_send_size.size;
            success = s_tcp_get_send_size.success;

            release_mutex__();
        }
    }

    return success;
}
/******************************************************************************/
static bool check_get_rtc_reply__(char const *str)
{
    /** Expect the line to be '+CCLK: "yy/mm/dd,hh:mm:ss+oo"'
     */
    if(
            ( strncmp(str, "+CCLK: ", 7) == 0 )
    )
    {
        str=&str[7];

        PRINTF("STR = %s\r\n", str);

        s_get_rtc_data.success = false;


        bool success=true;

        success &= eat_char(&str, '\"');
        success &= eat_u32(&str, &s_get_rtc_data.year);
        success &= eat_char(&str, '/');
        success &= eat_u32(&str, &s_get_rtc_data.month);
        success &= eat_char(&str, '/');
        success &= eat_u32(&str, &s_get_rtc_data.day);
        success &= eat_comma(&str);
        success &= eat_u32(&str, &s_get_rtc_data.hours);
        success &= eat_char(&str, ':');
        success &= eat_u32(&str, &s_get_rtc_data.minutes);
        success &= eat_char(&str, ':');
        success &= eat_u32(&str, &s_get_rtc_data.seconds);

        s_get_rtc_data.success = success;
    }

    return true;
}
/******************************************************************************/
bool Modem_get_rtc(uint32_t *p_timestamp)
{
    bool success=false;

    if(p_timestamp)
    {
        if( acquire_mutex__(1000U) )
        {
            s_get_rtc_data.success = false;

            if( run_command_ex__("AT+CCLK?", SEARCH_OK|SEARCH_ERROR, &check_get_rtc_reply__, NULL, 0U, 1000U) )
            {
                if(s_get_rtc_data.success)
                {
                    success = true;
                }

            }

            release_mutex__();


            struct tm tm;

            memset(&tm, 0, sizeof(tm));

            tm.tm_year = s_get_rtc_data.year + 100;     /* tm_year is years since 1900  */
            tm.tm_mon  = s_get_rtc_data.month - 1;      /* tm_mon range is 0 to 11 */
            tm.tm_mday = s_get_rtc_data.day;
            tm.tm_hour = s_get_rtc_data.hours;
            tm.tm_min  = s_get_rtc_data.minutes;
            tm.tm_sec  = s_get_rtc_data.seconds;

            *p_timestamp = timegm(&tm);             /* See README.md if your system lacks timegm(). */
        }

        PRINTF("Get RTC = %u.%u.%u.%u.%u.%u (%u) (%lu)\r\n",
                s_get_rtc_data.year,
                s_get_rtc_data.month,
                s_get_rtc_data.day,
                s_get_rtc_data.hours,
                s_get_rtc_data.minutes,
                s_get_rtc_data.seconds,
                s_get_rtc_data.success,
                *p_timestamp);
    }

    return success;
}
/******************************************************************************/
void ModemDrv_task(void const * argument)
{
    uint8_t ch;

    memset(&s_task_data, 0, sizeof(s_task_data));


    AlcTestCharSeq_clear(&s_current_command);
    AlcTestCharSeq_clear(&s_ipd);
    AlcTestCharSeq_clear(&s_open);
    AlcTestCharSeq_clear(&s_closed);

    AlcTestCharSeq_set(&s_ipd,    "+RECEIVE,");
    AlcTestCharSeq_set(&s_open,   "%d, CONNECT OK");
    AlcTestCharSeq_set(&s_closed, "%d, CLOSE OK");

    /* The Modem module is attached to UART6 */
    UART6_start();

    for(;;)
    {
        if( UART6_read( &ch, 1, 1000) > 0 )
        {
            process_rx_char__(ch);
        }
    }
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static bool run_command_ex__(char const *command_str, uint32_t search_mask, bool (*fn)(char const *str), uint8_t const* p_tx_buff, uint32_t tx_bufflen, uint32_t timeout_ms)
{
    bool succes=false;


    if( p_tx_buff == NULL )
    {
        tx_bufflen = 0U;
    }


    if( start_command(command_str, search_mask, fn, tx_bufflen) )
    {
        uint32_t start_time = osKernelSysTick();
        uint32_t bytes_send=0U;

        while( s_task_data.current_command.active )
        {
            osDelay(1);

            if( s_task_data.current_command.tx_data.ready_to_send )
            {
                PRINTF("ModemDrv - sending data\r\n");
                s_task_data.current_command.tx_data.ready_to_send = false;
                UART6_write(p_tx_buff, tx_bufflen, timeout_ms);
            }

            if( ( timeout_ms > 0 ) && ( ( osKernelSysTick() - start_time )  >= timeout_ms ) )
            {
                PRINTF("ModemDrv - Timeout waiting for command to complete\r\n");
                s_task_data.num_errors++;
                s_task_data.rx_state = RXST_IDLE;
                s_task_data.current_command.result = false;
                s_task_data.current_command.active = false;
                break;
            }
        }

        succes = s_task_data.current_command.result;
    }

    return succes;
}
/******************************************************************************/
static bool start_command(char const *command_str, uint32_t search_mask, bool (*fn)(char const *str), uint32_t tx_bufflen)
{
    bool succes=false;

    if(command_str)
    {
        if( !s_task_data.current_command.active )
        {
            PRINTF("ModemDrv - command '%s'\r\n", command_str);

            s_task_data.current_command.result = false;

            s_task_data.current_command.tx_data.ready_to_send = false;
            s_task_data.current_command.tx_data.tx_bufflen    = tx_bufflen;
            s_task_data.current_command.tx_data.tx_count      = 0U;

            AlcTestCharSeq_set(&s_current_command, command_str);
            s_task_data.current_command.search_mask = search_mask;
            s_task_data.current_command.fn          = fn;
            s_task_data.current_command.result      = false;
            s_task_data.rx_state                    = RXST_IDLE;
            s_task_data.current_command.active      = true;

            UART6_write(command_str, strlen(command_str), 100);
            UART6_write("\r\n", 2, 100);

            succes = true;
        }
    }

    return succes;
}
/******************************************************************************/
static void process_rx_char__(uint8_t ch)
{
    switch( s_task_data.rx_state )
    {
    case RXST_IDLE:
        /****************************************************************
         * Idle mode
         ***************************************************************/
        if( ch == ASCII_CR )
        {
            if( AlcTestCharSeq_is_found(&s_current_command) )
            {
                //
                UART3_write("<<<< FOUND CMD <<<<\r\n", 21, 100);
                AlcTestCharSeq_restart(&s_current_command);
                ReplyBuffer_reset();

                if( s_task_data.current_command.tx_data.tx_bufflen == 0U )
                {
                    /* No data to be transmitted -- so goto process command lines */
                    s_task_data.rx_state = RXST_COMMAND;
                }
                else
                {
                    /* There is data to be transmitted -- so now wait for modem
                     * to send '>' character before starting transmission.
                     */
                    s_task_data.rx_state = RXST_GT;
                }
            }
            else
            {
                UART3_write("\r\n", 2, 100);
            }


            AlcTestCharSeq_restart(&s_ipd);
            AlcTestCharSeq_restart(&s_open);
            AlcTestCharSeq_restart(&s_closed);
        }
        else if( ch == ASCII_LF )
        {
            // Ignore linefeed chars
        }
        else
        {
            UART3_write(&ch, 1, 100);

            AlcTestCharSeq_test_char(&s_current_command, ch);

            AlcTestCharSeq_test_char(&s_ipd, ch);
            if( AlcTestCharSeq_is_found(&s_ipd) )
            {
                AlcTestCharSeq_restart(&s_ipd);
                ipd_portnum_ = 0u;
                ipd_numbytes_ = 0u;
                s_task_data.rx_state = RXST_IPD_PORTNUM;
            }

            AlcTestCharSeq_test_char(&s_open, ch);
            if( AlcTestCharSeq_is_found(&s_open) )
            {
                AlcTestCharSeq_restart(&s_open);
                UART3_write("<<<<PORT OPEN>>>>", 17, 100);
                s_task_data.tcp_link_is_open = true;

                if( s_open.num == MODEM_CHANNEL_DATA_UPLOAD_CLIENT )
                {
                    /* The message is for channel 0 which is the HTTP server
                     */
                    xQueueReset(g_data_upload_client_rx_queueHandle);
                    HttpServer_connection_opened();
                }
            }

            AlcTestCharSeq_test_char(&s_closed, ch);
            if( AlcTestCharSeq_is_found(&s_closed) )
            {
                AlcTestCharSeq_restart(&s_closed);
                UART3_write("<<<<PORT CLOSED>>>>", 19, 100);
                s_task_data.tcp_link_is_open = false;

                if( s_open.num == 0U )
                {
                    /* The message is for channel 0 which is the HTTP server
                     */
                    HttpServer_connection_closed();
                }
            }
        }
        break;

    case RXST_COMMAND:
        /****************************************************************
         * A command is running
         ***************************************************************/
        if( ch == ASCII_CR )
        {
            UART3_write("  GOTLINE >>>>", 14, 100);
            UART3_write(replybuffer, strlen(replybuffer), 100);
            UART3_write("<<<<\r\n", 6, 100);

            if( s_task_data.current_command.fn )
            {
                s_task_data.current_command.fn(replybuffer);
            }

            if(
                ( s_task_data.current_command.search_mask & SEARCH_OK ) &&
                ( strcmp( replybuffer, "OK" ) == 0 )
            )
            {
                // Got OK - success!
                s_task_data.current_command.result = true;
                s_task_data.current_command.active = false;
                s_task_data.rx_state = RXST_IDLE;
            }
            else if(
                ( s_task_data.current_command.search_mask & SEARCH_FAIL ) &&
                ( strcmp( replybuffer, "FAIL" ) == 0 )
            )
            {
                // Got FAIL - error!
                s_task_data.current_command.result = false;
                s_task_data.current_command.active = false;
                s_task_data.rx_state = RXST_IDLE;
            }
            else if(
                ( s_task_data.current_command.search_mask & SEARCH_ERROR ) &&
                ( strcmp( replybuffer, "ERROR" ) == 0 )
            )
            {
                // Got ERROR - error!
                s_task_data.current_command.result = false;
                s_task_data.current_command.active = false;
                s_task_data.rx_state = RXST_IDLE;
            }
            else if(
                ( s_task_data.current_command.search_mask & SEARCH_BUSY_P ) &&
                ( strcmp( replybuffer, "busy p..." ) == 0 )
            )
            {
                // Got ERROR - error!
                s_task_data.current_command.result = false;
                s_task_data.current_command.active = false;
                s_task_data.rx_state = RXST_IDLE;
            }
            else if(
                ( s_task_data.current_command.search_mask & SEARCH_SHUT_OK ) &&
                ( strcmp( replybuffer, "SHUT OK" ) == 0 )
            )
            {
                // Got success!
                s_task_data.current_command.result = true;
                s_task_data.current_command.active = false;
                s_task_data.rx_state = RXST_IDLE;
            }
            else if(
                ( s_task_data.current_command.search_mask & SEARCH_CLOSE_OK ) &&
                ( strcmp( replybuffer, "5, CLOSE OK" ) == 0 )
            )
            {
                // Got success
                s_task_data.current_command.result = true;
                s_task_data.current_command.active = false;
                s_task_data.rx_state = RXST_IDLE;
            }
            else
            {
                // Ignore line
            }

            ReplyBuffer_reset();
        }
        else if( ch == ASCII_LF )
        {
            // Ignore linefeed chars
        }
        else
        {
            // Add character to current line
            ReplyBuffer_push_back(ch);
        }
        break;


    case RXST_GT:
        /****************************************************************
         * Wait for Modem to send a '>' (greater-than) character
         ***************************************************************/
        UART3_write(&ch, 1, 100);
        if( ch == '>' )
        {
            s_task_data.rx_state = RXST_TX_DATA;
            s_task_data.current_command.tx_data.ready_to_send = true;
            s_task_data.current_command.tx_data.tx_count = 0U;
        }
        break;

    case RXST_TX_DATA:
        /****************************************************************
         * Transmitting data to Modem -- it will be echoed back here
         ***************************************************************/
        UART3_write(&ch, 1, 100);
        s_task_data.current_command.tx_data.tx_count++;

        if( s_task_data.current_command.tx_data.tx_count >= s_task_data.current_command.tx_data.tx_bufflen )
        {
            s_task_data.rx_state = RXST_TX_DATA_COMPLETE;
            ReplyBuffer_reset();
        }
        break;

    case RXST_TX_DATA_COMPLETE:
        /****************************************************************
         * Data sent to Modem -- waiting for Modem to send "x, SEND OK"
         * string, where x is the channel number.
         ***************************************************************/
        UART3_write(&ch, 1, 100);
        if( ch == ASCII_CR )
        {
            if(
                    ( replybuffer[0] >= '0' ) &&
                    ( replybuffer[0] <= '9' ) &&
                    ( strcmp( &replybuffer[1], ", SEND OK" ) == 0 )
            )
            {
                //
                UART3_write("<<<< SEND COMPLETE >>>>\r\n", 25, 100);
                s_task_data.current_command.result = true;
                s_task_data.current_command.active = false;
                s_task_data.rx_state = RXST_IDLE;
            }

            ReplyBuffer_reset();
        }
        else if( ch == ASCII_LF )
        {
            // Ignore linefeed chars
        }
        else
        {
            ReplyBuffer_push_back(ch);
        }
        break;

    case RXST_IPD_PORTNUM:
        if( ( ch >= '0' ) && ( ch <= '9' ) )
        {
            ipd_portnum_ *= 10u;
            ipd_portnum_ += ( ch - '0' );
        }
        else if( ch == ',' )
        {
            s_task_data.rx_state = RXST_IPD_NUMBYTES;
        }
        else
        {
            s_task_data.rx_state = RXST_IDLE;
        }
        break;

    case RXST_IPD_NUMBYTES:
        if( ( ch >= '0' ) && ( ch <= '9' ) )
        {
            ipd_numbytes_ *= 10u;
            ipd_numbytes_ += ( ch - '0' );
        }
        else if( ch == ':' )
        {
            UART6_read( &ch, 1, 1000);
            UART6_read( &ch, 1, 1000);

            //UART3_write("[[[[", 4, 100);
            while( ipd_numbytes_ > 0U )
            {
                if( UART6_read( &ch, 1, 1000) > 0 )
                {
                    ipd_numbytes_--;

                    if( ipd_portnum_ == MODEM_CHANNEL_DATA_UPLOAD_CLIENT )
                    {
                        /* The data is for channel 0 which is the HTTP server
                         */
                        if( xQueueSendToBack(g_data_upload_client_rx_queueHandle, &ch, 1000) != pdPASS )
                        {
                            // Failed to post the message, even after 10 ticks.
                            //printf("ModemDrv - Failed to post the message\r\n");
                        }
                    }
                }
            }
            s_task_data.rx_state = RXST_IDLE;
            //osDelay(500u);
            //UART3_write("]]]]", 4, 100);
        }
        else
        {
            s_task_data.rx_state = RXST_IDLE;
        }
        break;


    default:
        /*
         * Should never get here
         */
        s_task_data.rx_state = RXST_IDLE;
        break;

    } /* switch() */
}
/******************************************************************************/
