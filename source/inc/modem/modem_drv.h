/**
 * @file  modem_drv.h
 * @brief Modem driver
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_MODEM_MODEM_DRV_H_
#define SOURCE_INC_MODEM_MODEM_DRV_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include <stdbool.h>
#include <stdint.h>




/*******************************************************************************
*                               DEFAULT CONFIGURATION
*******************************************************************************/




/*******************************************************************************
*                               DEFINES
*******************************************************************************/

#define MODEM_NW_REG_JOINED_NO          0U
#define MODEM_NW_REG_JOINED_HOME        1U
#define MODEM_NW_REG_JOINED_SEARCHING   2U
#define MODEM_NW_REG_JOINED_DENIED      3U
#define MODEM_NW_REG_JOINED_UNKNOWN     4U
#define MODEM_NW_REG_JOINED_ROAMING     5U




/*******************************************************************************
*                               DATA TYPES
*******************************************************************************/

#define SEARCH_OK       ( 1u << 0 )
#define SEARCH_FAIL     ( 1u << 2 )
#define SEARCH_ERROR    ( 1u << 3 )
#define SEARCH_BUSY_P   ( 1u << 4 )
#define SEARCH_SHUT_OK  ( 1u << 5 )
#define SEARCH_CLOSE_OK ( 1u << 6 )




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


bool Modem_run_command(char const *command_str,
                       uint32_t search_mask,
                       uint32_t timeout_ms);

bool Modem_run_command_tx_data(char const *command_str,
                               uint32_t search_mask,
                               uint8_t const* p_tx_buff,
                               uint32_t tx_bufflen,
                               uint32_t timeout_ms);

bool Modem_run_command_ex(char const *command_str,
                          uint32_t search_mask,
                          bool (*fn)(char const *str),
                          uint8_t const* p_tx_buff,
                          uint32_t tx_bufflen,
                          uint32_t timeout_ms);

bool Modem_run_command_parse_reply_u32(
        char const *command_str,
        char const *p_search_str,
        int32_t    *p_num,
        uint32_t    index,
        uint32_t search_mask,
        uint32_t timeout_ms);




void Modem_hard_reset(void);
bool Modem_soft_reset(void);

bool Modem_send_at(void);
bool Modem_status(void);



bool   Modem_enable_network_registration(void);
bool   Modem_get_network_registration(uint32_t *p_status);
bool   Modem_gprs_service_status(uint32_t *p_status);


bool   Modem_get_sim_ccid(void);
bool   Modem_is_password_required(void);
bool   Modem_unlock_sim(char const *p_pin);
bool   Modem_enable_gprs(bool on_off);
bool   Modem_enable_ntp(void);


double Modem_get_signal_strength(void);

bool Modem_get_ip_addr(char *dest, size_t destlen);
bool Modem_enable_mux(bool enable);


/* TCP functions */
bool Modem_tcp_write_buff(uint8_t channel, char const *p_buff, uint32_t bufflen, uint32_t timeout_ms);
bool Modem_tcp_write_str(uint8_t channel, char const *p_str, uint32_t timeout_ms);
bool Modem_tcp_open(uint8_t channel, char const *server, uint16_t port, uint32_t timeout_ms);
bool Modem_tcp_close(uint8_t channel, uint32_t timeout_ms);
bool Modem_tcp_get_send_size(uint8_t channel, uint32_t *p_size, uint32_t timeout_ms);


bool Modem_get_rtc(uint32_t *p_timestamp);


/** @brief The RTOS task that runs the Modem driver interface */
void ModemDrv_task(void const * argument);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_MODEM_MODEM_DRV_H_ */
