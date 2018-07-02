/**
 * @file  sensor_node_list.c
 * @brief Sensor Node List -- Maintains a list of SensorNode objects.
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 *
 *
 * This module maintains a list of known SensorNode objects.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "sensor_node_list.h"

#include "alc_assert.h"
#include "contiki.h"
#include "net/ipv6/uip-ds6.h"


#define DEBUG DEBUG_NONE
#include "net-debug.h"
#include "uip-debug.h"




#ifndef SENSOR_NODE_LIST_SIZE
#error "SENSOR_NODE_LIST_SIZE has not been defined in contiki-conf.h"
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

static SensorNode s_node_list[SENSOR_NODE_LIST_SIZE];




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static bool find_nodes_index__(SensorNode const* p_sensor_node, uint32_t *p_index);




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
void SNL_init(void)
{
    PRINTF("SNL_init() -- todo!!\r\n");

    for(uint32_t ii=0; ii<SENSOR_NODE_LIST_SIZE; ii++)
    {
        SensorNode_init(&s_node_list[ii]);
    }
}
/******************************************************************************/
uint32_t SNL_get_size(void)
{
    uint32_t size=0U;

    for(uint32_t ii=0; ii<SENSOR_NODE_LIST_SIZE; ii++)
    {
        if( s_node_list[ii].is_used )
        {
            size++;
        }
    }

    return size;
}
/******************************************************************************/
uint32_t SNL_get_max_size(void)
{
    return ( SENSOR_NODE_LIST_SIZE - 1 );
}
/******************************************************************************/
SensorNode* SNL_find(uip_ipaddr_t const *p_ipaddr, bool create_if_none, bool *p_was_created)
{
    SensorNode *p_sensor_node=NULL;

    if(p_was_created)
    {
        *p_was_created = false;
    }

    if(p_ipaddr)
    {
        int unused_idx=-1;

        /* Search through the list
         */
        for(uint32_t ii=0; ii<SENSOR_NODE_LIST_SIZE; ii++)
        {
            if( s_node_list[ii].is_used )
            {
                /* The element is in use -- test if the IP address matches
                 */
                if( uip_ipaddr_cmp(&s_node_list[ii].ipaddr, p_ipaddr)  )
                {
                    /* Found it -- set return pointer and exit for() loop */
                    //PRINTF("Found element at index %d\r\n", ii);
                    p_sensor_node = &s_node_list[ii];
                    break;
                }
            }
            else if(unused_idx == -1)
            {
                /* Found an unused element -- remember the index in case we need
                 * to create a new entry later.
                 */
                unused_idx = ii;
            }
        }

        if( ( p_sensor_node == NULL ) && (create_if_none) )
        {
            /* The IP address is not in the list, and we have been asked to
             * create a new entry if none is found
             */
            if( ( unused_idx >= 0 ) && ( unused_idx < SENSOR_NODE_LIST_SIZE ) )
            {
                /* Create new entry here */
                p_sensor_node = &s_node_list[unused_idx];

                PRINTF("Creating entry at index %d for ", unused_idx);
                PRINT6ADDR(p_ipaddr);
                PRINTF("\r\n");

                SensorNode_init(p_sensor_node);

                uip_ipaddr_copy(&p_sensor_node->ipaddr, p_ipaddr);

                p_sensor_node->is_used = true;

                if(p_was_created)
                {
                    *p_was_created = true;
                }
            }
            else
            {
                /* List is full -- don't do anything */
                PRINTF("List is full -- can't add new item\r\n");
            }
        }
    }

    return p_sensor_node;
}
/******************************************************************************/
void SNL_for_each_node(bool (*fn)(uint32_t index, SensorNode const *p_node))
{
    if(fn)
    {
        /* Search through the list
         */
        for(uint32_t ii=0; ii<SENSOR_NODE_LIST_SIZE; ii++)
        {
            if( s_node_list[ii].is_used )
            {
                if(!fn(ii, &s_node_list[ii]))
                {
                    break;
                }
            }
        }
    }
}
/******************************************************************************/
bool SNL_is_in_list(SensorNode const* p_sensor_node)
{
    return find_nodes_index__(p_sensor_node, NULL);
}
/******************************************************************************/
SensorNode* SNL_find_first_active_node(void)
{
    SensorNode *p_first_node=NULL;

    for(uint32_t ii=0U; ii<SENSOR_NODE_LIST_SIZE; ii++)
    {
        if(s_node_list[ii].is_used)
        {
            /* Found the first used node */
            p_first_node = &s_node_list[ii];
            break;
        }
    }

    return p_first_node;
}
/******************************************************************************/
SensorNode* SNL_find_next_active_node(SensorNode const *p_sensor_node, bool wrap_search)
{
    uint32_t start_index;
    SensorNode *p_next_active_node=NULL;


    /* find the location of the pointer in the array */
    if( find_nodes_index__(p_sensor_node, &start_index) )
    {
        /* The pointer is in the array...
         * Increment index to the next element in the array.
         */
        start_index++;

        if( ( start_index >= SENSOR_NODE_LIST_SIZE ) && ( wrap_search ) )
        {
            /* loop bck to start of array */
            start_index = 0U;
        }
    }
    else
    {
        /* The pointer was not in the array, or it was NULL...
         * Start from the beginning of the array.
         */
        start_index = 0U;
    }


    if( start_index < SENSOR_NODE_LIST_SIZE )
    {
        bool got_node=false;

        /* Search through the list
         */
        for(uint32_t ii=start_index; ii<SENSOR_NODE_LIST_SIZE; ii++)
        {
            if(s_node_list[ii].is_used)
            {
                /* Found the right index */
                p_next_active_node = &s_node_list[ii];
                got_node = true;
                break;
            }
        }


        if( (!got_node) && (start_index>0U) && (wrap_search) )
        {
            for(uint32_t ii=0U; ii<start_index; ii++)
            {
                if(s_node_list[ii].is_used)
                {
                    /* Found the right index */
                    p_next_active_node = &s_node_list[ii];
                    got_node = true;
                    break;
                }
            }
        }
    }


    return p_next_active_node;
}
/******************************************************************************/
uint32_t SNL_remove_deleted_nodes(void)
{
    uint32_t count_deleted=0U;

    /* Search through the list
     */
    for(uint32_t ii=0; ii<SENSOR_NODE_LIST_SIZE; ii++)
    {
        if(
                ( s_node_list[ii].is_used ) &&
                ( s_node_list[ii].flags.for_deleting )
        )
        {
            /* delete here */
            if( SensorNode_destroy(&s_node_list[ii]) )
            {
                PRINTF("Node deleted ");
                PRINT6ADDR(p_ipaddr);
                PRINTF("\r\n");

                s_node_list[ii].is_used = false;

                count_deleted++;
            }
        }
    }

    return count_deleted;
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static bool find_nodes_index__(SensorNode const* p_sensor_node, uint32_t *p_index)
{
    bool is_in_list=false;

    if(p_sensor_node)
    {
        /* Search through the list
         */
        for(uint32_t ii=0; ii<SENSOR_NODE_LIST_SIZE; ii++)
        {
            /* test if pointer matches */
            if( p_sensor_node == &s_node_list[ii] )
            {
                /* Found the right index */
                is_in_list = true;

                if(p_index)
                {
                    *p_index = ii;
                }
                break;
            }
        }
    }

    return  is_in_list;
}
/******************************************************************************/
