/**
 * @file  modem_ctrl.h
 * @brief Modem controller (rtos task)
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_MODEM_MODEM_CTRL_H_
#define SOURCE_INC_MODEM_MODEM_CTRL_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "gps_data.h"




/*******************************************************************************
*                               DEFAULT CONFIGURATION
*******************************************************************************/

// @todo
#define MODEM_IS_CELLULAR   1
#if MODEM_IS_CELLULAR
#define MODEM_HAS_GPS           0   /**< Using GPS module on Modem board, [1=yes, 0=no] */
#define USING_GPS_TIME_CTRL     0   /**< Get time from GPS module, [1=yes, 0=no] */
#define USING_NTP_TIME_CTRL     0   /**< Get time from RTC on Modem module, [1=yes, 0=no] */
#define USING_SERVER_TIME_CTRL  1   /**< Get time from Cloud Server, [1=yes, 0=no] */
#else
// ESP8266 Wifi module
#define MODEM_HAS_GPS       0
#endif




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


uint32_t ModemCtrl_get_state(void);

/** @brief test if the modem is read for commands
 *
 * @return true/false the modem can execute commands
 */
bool ModemCtrl_modem_is_ready(void);
bool ModemCtrl_gps_is_ready(void);
bool ModemCtrl_get_gps_info(GpsSentence *p_inf, uint32_t *p_timestamp);


void ModemCtrl_restart_modem(void);


void ModemCtrl_conf_has_been_changed(void);
bool ModemCtrl_check_conf_has_changed(void);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_MODEM_MODEM_CTRL_H_ */
