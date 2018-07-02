/**
 * @file  data_upload_msg.h
 * @brief Utility functions for uploading data to the cloud
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_NET_DATA_UPLOAD_MSG_H_
#define SOURCE_INC_NET_DATA_UPLOAD_MSG_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "sensor_data.h"
#include "sensor_node.h"




/*******************************************************************************
*                               DEFAULT CONFIGURATION
*******************************************************************************/




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


void prepare_security_string_msg(char *dest, uint32_t len);
void prepare_gateway_msg(char *dest, uint32_t len);
void prepare_node_long_msg(char *dest, uint32_t len, SensorNode const *p_sensor_node);
void prepare_node_msg(char *dest, uint32_t len, SensorNode const *p_sensor_node);
void prepare_data_msg(char *dest, uint32_t len, struct SensorData *p_sensor_data);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_NET_DATA_UPLOAD_MSG_H_ */
