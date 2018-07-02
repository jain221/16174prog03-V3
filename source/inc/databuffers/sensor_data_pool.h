/**
 * @file  sensor_data_pool.h
 * @brief A pool of unused SensorData objects.
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_DATABUFFERS_SENSOR_DATA_POOL_H_
#define SOURCE_INC_DATABUFFERS_SENSOR_DATA_POOL_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "sensor_data.h"




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


/** @brief Initialise the pool
 *
 * @note Must before other functions called
 */
void SensorDataPool_init(void);


uint32_t SensorDataPool_get_size(void);
uint32_t SensorDataPool_get_max_size(void);
uint32_t SensorDataPool_get_min_size(void);
void     SensorDataPool_reset_min_size(void);


/** @brief Get a SensorData object from the memory pool
 *
 * @return A pointer to the object, or NULL if pool is empty
 */
struct SensorData* SensorDataPool_get(void);


/** @brief Return a SensorData object to the memory pool
 *
 * @param p_data  A pointer to the object
 */
void SensorDataPool_return(struct SensorData *p_data);


void SensorDataPool_check_links(void);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_DATABUFFERS_SENSOR_DATA_POOL_H_ */
