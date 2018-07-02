/**
 * @file  sensor_node_list.h
 * @brief Sensor Node List -- Maintains a list of SensorNode objects.
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 *
 *
 * This module maintains a list of known SensorNode objects.
 */

#ifndef SOURCE_INC_DATABUFFERS_SENSOR_NODE_LIST_H_
#define SOURCE_INC_DATABUFFERS_SENSOR_NODE_LIST_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
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


void SNL_init(void);

uint32_t SNL_get_size(void);
uint32_t SNL_get_max_size(void);

SensorNode* SNL_find(uip_ipaddr_t const *p_ipaddr, bool create_if_none, bool *p_was_created);

void SNL_for_each_node(bool (*fn)(uint32_t index, SensorNode const *p_node));


bool SNL_is_in_list(SensorNode const* p_sensor_node);
SensorNode* SNL_find_first_active_node(void);
SensorNode* SNL_find_next_active_node(SensorNode const *p_sensor_node, bool wrap_search);


uint32_t SNL_remove_deleted_nodes(void);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_DATABUFFERS_SENSOR_NODE_LIST_H_ */
