/**
 * @file  sensor_data_list.h
 * @brief A struct for maintaining a linked-list of SensorData objects.
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_DATABUFFERS_SENSOR_DATA_LIST_H_
#define SOURCE_INC_DATABUFFERS_SENSOR_DATA_LIST_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "sensor_data.h"

#include <stdbool.h>




/*******************************************************************************
*                               DEFAULT CONFIGURATION
*******************************************************************************/




/*******************************************************************************
*                               DEFINES
*******************************************************************************/




/*******************************************************************************
*                               DATA TYPES
*******************************************************************************/

typedef struct {
    struct SensorData *p_first;     /* The first object in the list. */
    struct SensorData *p_last;      /* The last object in the list. */
    uint32_t size;                  /* The number of objects in the list. */
} SensorDataList;




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


void SensorDataList_init(SensorDataList *p_self);


bool SensorDataList_is_empty(SensorDataList const *p_self);
uint32_t SensorDataList_get_size(SensorDataList const *p_self);

bool SensorDataList_insert(SensorDataList *p_self, struct SensorData *p_data);

uint32_t SensorDataList_max_pop_len(SensorDataList const *p_self, uint32_t limit);
struct SensorData* SensorDataList_pop_front(SensorDataList *p_self);
void SensorDataList_push_back(SensorDataList *p_self, struct SensorData *p_data);

void SensorDataList_check_links(SensorDataList const *p_self);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_DATABUFFERS_SENSOR_DATA_LIST_H_ */
