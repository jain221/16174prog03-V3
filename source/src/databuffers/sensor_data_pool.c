/**
 * @file  sensor_data_pool.c
 * @brief A pool of unused SensorData objects.
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "sensor_data_pool.h"

#include "sensor_data_list.h"

#include "contiki.h"
#include "net/ipv6/uip-ds6.h"

#include "alc_assert.h"
#include "alc_logger.h"
#include "cmsis_os.h"




#ifndef SENSOR_DATA_POOL_SIZE
#error "SENSOR_DATA_POOL_SIZE has not been defined in contiki-conf.h"
#endif




/*******************************************************************************
*                               LOCAL DEFINES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL CONSTANTS
*******************************************************************************/




/*******************************************************************************
*                               LOCAL DATA TYPES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL TABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL GLOBAL VARIABLES
*******************************************************************************/

extern osMutexId g_sensor_data_pool_mutexHandle;


/** @brief Pointers for the pool. */
static SensorDataList s_pointers;
static uint32_t s_min_size=SENSOR_DATA_POOL_SIZE;


/** @brief The pool of objects. */
static struct SensorData s_sensor_data_pool[SENSOR_DATA_POOL_SIZE];




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
void SensorDataPool_init(void)
{
    /*
     * Using mutex to protect data pool from access by multiple threads
     */
    if( osMutexWait(g_sensor_data_pool_mutexHandle, osWaitForever) == osOK )
    {
        s_pointers.p_first  = &s_sensor_data_pool[0];
        s_pointers.p_last   = &s_sensor_data_pool[SENSOR_DATA_POOL_SIZE-1];
        s_pointers.size     = SENSOR_DATA_POOL_SIZE;
        s_min_size = SENSOR_DATA_POOL_SIZE;

        for(uint32_t ii=0U; ii<SENSOR_DATA_POOL_SIZE; ii++)
        {
            /* Set the previous pointer */
            if( ii == 0U )
            {
                s_sensor_data_pool[ii].p_prev = NULL;
            }
            else
            {
                s_sensor_data_pool[ii].p_prev = &s_sensor_data_pool[ii-1];
            }

            /* Set the next pointer */
            if( ii < ( SENSOR_DATA_POOL_SIZE - 1U ) )
            {
                s_sensor_data_pool[ii].p_next = &s_sensor_data_pool[ii+1];
            }
            else
            {
                s_sensor_data_pool[ii].p_next = NULL;
            }

            /* Initialise other data in structure */
            s_sensor_data_pool[ii].ts_seconds   = 0U;
            s_sensor_data_pool[ii].ts_hundreths = 0U;
        }

        /* release mutex */
        osMutexRelease(g_sensor_data_pool_mutexHandle);
    }
}
/******************************************************************************/
uint32_t SensorDataPool_get_size(void)
{
    return s_pointers.size;
}
/******************************************************************************/
uint32_t SensorDataPool_get_max_size(void)
{
    return SENSOR_DATA_POOL_SIZE;
}
/******************************************************************************/
uint32_t SensorDataPool_get_min_size(void)
{
    return s_min_size;
}
/******************************************************************************/
void SensorDataPool_reset_min_size(void)
{
    s_min_size = s_pointers.size;
}
/******************************************************************************/
/* Get method will pull the first node in the list.
 */
struct SensorData* SensorDataPool_get(void)
{
    struct SensorData *p_data=NULL;

    /*
     * Using mutex to protect data pool from access by multiple threads
     */
    if( osMutexWait(g_sensor_data_pool_mutexHandle, 1000) == osOK )
    {
        p_data = SensorDataList_pop_front(&s_pointers);

        if(p_data)
        {
            if( s_pointers.size < s_min_size )
            {
                /* New minimum size */
                s_min_size = s_pointers.size;
            }

            if( s_pointers.size == 0U )
            {
                /* pool is now empty */
                AlcLogger_log_warning("SensorDataPool is empty.");
            }
        }


        /* release mutex */
        osMutexRelease(g_sensor_data_pool_mutexHandle);
    }
    return p_data;
}
/******************************************************************************/
/* Return method will append the node to the end of the list.
 */
void SensorDataPool_return(struct SensorData *p_data)
{
    /*
     * Using mutex to protect data pool from access by multiple threads
     */
    if( osMutexWait(g_sensor_data_pool_mutexHandle, 1000) == osOK )
    {
        /* pointer should be in the data pool memory region */
        ALC_ASSERT( p_data >= &s_sensor_data_pool[0]);
        ALC_ASSERT( p_data <  &s_sensor_data_pool[SENSOR_DATA_POOL_SIZE]);

        SensorDataList_push_back(&s_pointers, p_data);

        /* release mutex */
        osMutexRelease(g_sensor_data_pool_mutexHandle);
    }
}
/******************************************************************************/
void SensorDataPool_check_links(void)
{
    SensorDataList_check_links(&s_pointers);
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/
