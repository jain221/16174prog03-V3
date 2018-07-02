/**
 * @file  sensor_node.c
 * @brief A struct to maintain info about a remote sensor node.
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "sensor_node.h"

#include "alc_assert.h"
#include "cmsis_os.h"
#include "sensor_data_pool.h"
#include "sys/clock.h"




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

/* This module uses this mutex to control access to the objects from multiple
 * threads.
 */
extern osMutexId g_sensor_node_mutexHandle;




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static uint32_t calc_max_pop_len__(SensorNode const *p_self, uint32_t limit);
static void flush_data_stream__(SensorNode const *p_self);




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
void SensorNode_init(SensorNode *p_self)
{
    if(p_self)
    {
        /*
         * Using mutex to protect sensor-node objects from access by multiple threads
         */
        if( osMutexWait(g_sensor_node_mutexHandle, 1000) == osOK )
        {
            memset(p_self, 0, sizeof(SensorNode));

            SensorDataList_init(&p_self->data_list);

            SensorNode_update_last_msg_rx_time(p_self);

            p_self->lat = 0.0f;
            p_self->lon = 0.0f;

            p_self->flags.is_dirty = true;

            /* release mutex */
            osMutexRelease(g_sensor_node_mutexHandle);
        }
    }
}
/******************************************************************************/
bool SensorNode_destroy(SensorNode *p_self)
{
    bool success=false;

    if(p_self)
    {
        /*
         * Using mutex to protect sensor-node objects from access by multiple threads
         */
        if( osMutexWait(g_sensor_node_mutexHandle, 1000) == osOK )
        {
            /* delete all data and return it to the store */
            flush_data_stream__(p_self);

            /* release mutex */
            osMutexRelease(g_sensor_node_mutexHandle);

            success = true;
        }
    }

    return success;
}
/******************************************************************************/
bool SensorNode_reset_data_stream(SensorNode *p_self, uint16_t id16, uint32_t seq32)
{
    bool success=false;

    if(p_self)
    {
        /*
         * Using mutex to protect sensor-node objects from access by multiple threads
         */
        if( osMutexWait(g_sensor_node_mutexHandle, 1000) == osOK )
        {
            flush_data_stream__(p_self);

            p_self->id16             = id16;
            p_self->wrong_id16_count = 0U;
            p_self->front_seq32      = seq32;

            /* release mutex */
            osMutexRelease(g_sensor_node_mutexHandle);
        }
    }
    return success;
}
/******************************************************************************/
bool SensorNode_add_data(SensorNode *p_self, struct SensorData *p_sensor_data)
{
    bool success=false;

    if( (p_self) && (p_sensor_data) )
    {
        /*
         * Using mutex to protect sensor-node objects from access by multiple threads
         */
        if( osMutexWait(g_sensor_node_mutexHandle, 1000) == osOK )
        {
            /* Use insert as the data needs to be sorted */
            success = SensorDataList_insert(&p_self->data_list, p_sensor_data);

            /* release mutex */
            osMutexRelease(g_sensor_node_mutexHandle);
        }
    }
    return success;
}
/******************************************************************************/
struct SensorData* SensorNode_remove_data(SensorNode *p_self)
{
    struct SensorData *p_sensor_data=NULL;

    if(p_self)
    {
        /*
         * Using mutex to protect sensor-node objects from access by multiple threads
         */
        if( osMutexWait(g_sensor_node_mutexHandle, 1000) == osOK )
        {
            // todo
            p_sensor_data = SensorDataList_pop_front(&p_self->data_list);

            if(p_sensor_data)
            {
                p_self->front_seq32 = ( p_sensor_data->seq32 + 1 );
            }

            /* release mutex */
            osMutexRelease(g_sensor_node_mutexHandle);
        }
    }

    return p_sensor_data;
}
/******************************************************************************/
uint32_t SensorNode_get_data_size(SensorNode *p_self)
{
    uint32_t count=0U;

    if(p_self)
    {
        /*
         * Using mutex to protect sensor-node objects from access by multiple threads
         */
        if( osMutexWait(g_sensor_node_mutexHandle, 1000) == osOK )
        {
            count = SensorDataList_get_size(&p_self->data_list);

            /* release mutex */
            osMutexRelease(g_sensor_node_mutexHandle);
        }
    }

    return count;
}
/******************************************************************************/
uint32_t SensorNode_max_pop_len(SensorNode const *p_self, uint32_t limit)
{
    uint32_t count=0U;

    if(p_self)
    {
        /*
         * Using mutex to protect sensor-node objects from access by multiple threads
         */
        if( osMutexWait(g_sensor_node_mutexHandle, 1000) == osOK )
        {
            count = calc_max_pop_len__(p_self, limit);


            /* release mutex */
            osMutexRelease(g_sensor_node_mutexHandle);
        }
    }

    return count;
}
/******************************************************************************/
uint32_t SensorNode_received_to_seq32(SensorNode const *p_self)
{
    uint32_t seq32=0U;

    if(p_self)
    {
        /*
         * Using mutex to protect sensor-node objects from access by multiple threads
         */
        if( osMutexWait(g_sensor_node_mutexHandle, 1000) == osOK )
        {
            seq32 =  p_self->front_seq32;
            seq32 += calc_max_pop_len__(p_self, UINT32_MAX);
            seq32 -= 1U;

            /* release mutex */
            osMutexRelease(g_sensor_node_mutexHandle);
        }
    }

    return seq32;
}
/******************************************************************************/
char const* SensorNode_get_status_string(SensorNode const *p_self)
{
    if(p_self)
    {
        if(p_self->flags.for_deleting)
        {
            return "deleting";
        }

        if(p_self->flags.is_stale)
        {
            return "stale";
        }

        return "ok";
    }

    return "?";
}
/******************************************************************************/
void SensorNode_mark_as_dirty(SensorNode *p_self)
{
    if(p_self)
    {
        p_self->flags.is_dirty = true;
    }
}
/******************************************************************************/
void SensorNode_clear_is_dirty(SensorNode *p_self)
{
    if(p_self)
    {
        p_self->flags.is_dirty = false;
    }
}
/******************************************************************************/
bool SensorNode_is_dirty(SensorNode const *p_self)
{
    if(p_self)
    {
        return (p_self->flags.is_dirty) ? true : false;
    }

    return false;
}
/******************************************************************************/
void SensorNode_mark_as_stale(SensorNode *p_self)
{
    if(p_self)
    {
        p_self->flags.is_stale = true;
    }
}
/******************************************************************************/
void SensorNode_update_last_msg_rx_time(SensorNode *p_self)
{
    if(p_self)
    {
        p_self->flags.is_stale   = false;
        p_self->last_msg_rx_time = clock_seconds();
    }
}
/******************************************************************************/
uint32_t SensorNode_seconds_since_last_msg_rx(SensorNode const *p_self)
{
    uint32_t tdiff=0U;

    if(p_self)
    {
        tdiff = ( clock_seconds() - p_self->last_msg_rx_time );
    }

    return tdiff;
}
/******************************************************************************/
void SensorNode_mark_for_deletion(SensorNode *p_self)
{
    if(p_self)
    {
        p_self->flags.for_deleting = true;
    }
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static uint32_t calc_max_pop_len__(SensorNode const *p_self, uint32_t limit)
{
    uint32_t count=0U;

    if(
            ( p_self ) &&
            ( p_self->data_list.p_first ) &&
            ( p_self->data_list.p_first->seq32 == p_self->front_seq32 )
    )
    {
        count = SensorDataList_max_pop_len(&p_self->data_list, limit);
    }

    return count;
}
/******************************************************************************/
static void flush_data_stream__(SensorNode const *p_self)
{
    uint32_t count=0U;

    if( p_self )
    {
        /* while the data list is not empty... */
        while( !SensorDataList_is_empty(&p_self->data_list) )
        {
            /* Remove data from the list */
            struct SensorData *p_data = SensorDataList_pop_front(&p_self->data_list);

            /* Do sanity check */
            ALC_ASSERT(p_data != NULL);

            /* Return data to the pool */
            SensorDataPool_return(p_data);
        }
    }

    return count;
}
/******************************************************************************/
