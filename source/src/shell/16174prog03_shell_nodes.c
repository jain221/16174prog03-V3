/**
 * @file  16174prog03_shell_nodes.c
 * @brief Shell commands for 16174prog03
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "16174prog03_shell_nodes.h"

#include <stdio.h>
#include <time.h>

#include "contiki.h"

#include "net/ipv6/uip-ds6.h"
#include "node-id.h"
#include "sensor_data_pool.h"
#include "sensor_node_list.h"
#include "shell.h"


/* Set to nonzero to display extra info on FIFO sequence numbers */
#define DEBUG_FIFO_SEQUENCE_NUMBERS     0


#define DEBUG DEBUG_NONE
#include "net-debug.h"
#include "uip-debug.h"




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




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/

static bool display_node_info__(uint32_t index, SensorNode const *p_node);




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/


/******************************************************************************/
PROCESS(C16174prog03_shell_nodes_process, "nodes");
SHELL_COMMAND(nodes_command,
          "nodes",
          "nodes: print nodes debug info",
          &C16174prog03_shell_nodes_process);
/******************************************************************************/
PROCESS_THREAD(C16174prog03_shell_nodes_process, ev, data)
{
    PROCESS_BEGIN();

    printf("nodes\r\n\r\n");

    printf("  size     = %lu\r\n", SNL_get_size());
    printf("  max size = %lu\r\n", SNL_get_max_size());

    SNL_for_each_node(&display_node_info__);

    printf("  free pool size   = %u\r\n", SensorDataPool_get_size());
    printf("  free pool lowest = %u\r\n", SensorDataPool_get_min_size());

    printf("\r\nOK\r\n\r\n");

    PROCESS_END();
}
/******************************************************************************/
void C16174prog03_shell_nodes_init(void)
{
    shell_register_command(&nodes_command);
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
static bool display_node_info__(uint32_t index, SensorNode const *p_node)
{
    if(p_node)
    {
#if DEBUG_FIFO_SEQUENCE_NUMBERS
        uint32_t first_seq32 = ( p_node->data_list.p_first ) ? p_node->data_list.p_first->seq32 : 0U;
        uint32_t last_seq32  = ( p_node->data_list.p_last  ) ? p_node->data_list.p_last->seq32  : 0U;
#endif

        printf("  #%lu,", index);
        uip_debug_ipaddr_print(&p_node->ipaddr);
#if DEBUG_FIFO_SEQUENCE_NUMBERS
        printf(",%s,id=%04x,rx=%u,size=%u,seq=%u/%u/%u/%u\r\n",
                SensorNode_get_status_string(p_node),
                p_node->id16,
                p_node->num_rx_packets,
                SensorNode_get_data_size(p_node),
                p_node->front_seq32,
                first_seq32,
                last_seq32,
                SensorNode_received_to_seq32(p_node));
#else
        printf(",%s,id=%04Xh,waiting=%u,rx=%u,size=%u,front=%u,seq=%u,end=%u,%u mA RMS,%u s\r\n",
                SensorNode_get_status_string(p_node),
                p_node->id16,
                p_node->num_samples_waiting,
                p_node->num_rx_packets,
                SensorNode_get_data_size(p_node),
                p_node->front_seq32,
                SensorNode_received_to_seq32(p_node),
                p_node->end_seq32,
                p_node->bulb_current_ma_rms,
                SensorNode_seconds_since_last_msg_rx(p_node));
#endif
    }

    return true;
}
/******************************************************************************/
