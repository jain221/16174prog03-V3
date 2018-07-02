/**
 * @file  project-conf.h
 * @brief Specific settings for the 16174prog03 project.
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_PROJECT_CONF_H_
#define SOURCE_INC_PROJECT_CONF_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/




/*******************************************************************************
*                               DEFAULT CONFIGURATION
*******************************************************************************/

#define ALC_USING_RADIO_SETUP       1

#define SENSOR_NODE_LIST_SIZE       50
#define SENSOR_DATA_POOL_SIZE       10000


#define LOG_CONF_ENABLED            1


/** @brief Actions to perform in systick IRQ handler.
 */
#define ON_SYSTICK_HANDLER()        do{}while(0)


/** @brief Prefix for the network.
 */
#define PREFIX_ADDR0                UIP_DS6_DEFAULT_PREFIX
#define PREFIX_ADDR1                0x0000
#define PREFIX_ADDR2                0x0000
#define PREFIX_ADDR3                0x0000
#define PREFIX_ADDR4                0x0000
#define PREFIX_ADDR5                0x0000
#define PREFIX_ADDR6                0x0000
#define PREFIX_ADDR7                0x0000




/* Definitions for alc_pole_cluster_ctrl_server.c */
#define ALC_PCCS_SEND_INTERVAL      (60 * CLOCK_SECOND)     /**< Every minute */
#if 0
/* values used in-house for development */

#if PRODUCTION_RELEASE
#error "Can't use development values in PRODUCTION_RELEASE"
#endif

#define ALC_PCCS_COLLECTING_TIME    ( 1U * 60U * (CLOCK_SECOND) )   /**< 1 minute */
#define ALC_PCCS_RECEIVING_TIME     ( 1U * 60U * (CLOCK_SECOND) )   /**< 1 minute */

#else
/* values used for production */

#define ALC_PCCS_COLLECTING_TIME    ( 10U * 60U * (CLOCK_SECOND) )  /**< 10 minutes */
#define ALC_PCCS_RECEIVING_TIME     ( 50U * 60U * (CLOCK_SECOND) )  /**< 50 minutes */

#endif




/* Definitions for alc_pole_data_link_root.c module */
#define ALC_PDLR_CMD_1ST_DELAY_MS           ( 5U * CLOCK_SECOND )   /**< 5 seconds */
#define ALC_PDLR_CMD_REPEAT_DELAY_MS        ( 20U * CLOCK_SECOND )  /**< 20 seconds */
#define ALC_PDLR_CMD_MAX_TX_COUNT           10U




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




#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_PROJECT_CONF_H_ */
