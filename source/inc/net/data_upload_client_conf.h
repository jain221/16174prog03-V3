/**
 * @file  data_upload_client_conf.h
 * @brief Configuration for data_upload_client module
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd., Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_NET_DATA_UPLOAD_CLIENT_CONF_H_
#define SOURCE_INC_NET_DATA_UPLOAD_CLIENT_CONF_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/




/** @weakgroup alc_data_upload_client ALC Data Upload Client
 *  @{
 */



/*******************************************************************************
*                               DEFAULT CONFIGURATION
*******************************************************************************/




/*******************************************************************************
*                               DEFINES
*******************************************************************************/

#if 0
/* values used in-house for development */

#if PRODUCTION_RELEASE
#error "PRODUCTION_RELEASE"
#endif

#define DUC_GATEWAY_MSG_INTERVAL_S      ( 10U * 60U )           /**< Every 10 minutes */
#define DUC_NODE_LONG_MSG_INTERVAL_S    ( 10U * 60U )           /**< Every 10 minutes */
#define DUC_RUN_CHECKS_INTERVAL_S       ( 30U )                 /**< Every 30 seconds */
#define DUC_LOST_CONN_INTERVAL_S        ( 4U * 60U )            /**< Every 4 minutes */
#define DUC_LOOSING_CONN_INTERVAL_S     ( 2U * 60U )            /**< Every 2 minutes */

#else
/* values used for production */

#define DUC_GATEWAY_MSG_INTERVAL_S      ( 60U )                 /**< Every minute */
#define DUC_NODE_LONG_MSG_INTERVAL_S    ( 10U * 60U )           /**< Every 10 minutes */
#define DUC_RUN_CHECKS_INTERVAL_S       ( 60U * 60U )           /**< Hourly */
#define DUC_LOST_CONN_INTERVAL_S        ( 20U * 60U * 60U )     /**< 20 hours */
#define DUC_LOOSING_CONN_INTERVAL_S     ( 5U * 60U * 60U )      /**< 5 hours */

#endif




#define DUC_RETRY_OPEN_PERIOD_MS        ( 30000U )              /**< Retry open TCP every 30 seconds */
#define DUC_RETRY_OPEN_LIMIT            ( 20U )                 /**< Max attempts before reset modem */




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




#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/



/** @} end of grouping
 */


#endif /* SOURCE_INC_NET_DATA_UPLOAD_CLIENT_CONF_H_ */
