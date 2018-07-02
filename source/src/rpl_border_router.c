/**
 * @file  rpl_border_router.c
 * @brief The RPL border router process
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#if RPL_WITH_NON_STORING
#include "net/rpl/rpl-ns.h"
#endif /* RPL_WITH_NON_STORING */
#include "net/netstack.h"
#include "dev/button-sensor.h"
#include "dev/slip.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"




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

static uip_ipaddr_t s_prefix64;
static volatile uint8_t s_prefix64_is_set=0;
static uip_ipaddr_t s_global_ipaddr;


PROCESS(border_router_process, "Border router process");




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
void set_prefix_64(uip_ipaddr_t *prefix_64)
{
    rpl_dag_t *dag;

    memcpy( &s_prefix64, prefix_64, 16);
    memcpy( &s_global_ipaddr, prefix_64, 16);

    s_prefix64_is_set = 1;
    uip_ds6_set_addr_iid( &s_global_ipaddr, &uip_lladdr);
    uip_ds6_addr_add( &s_global_ipaddr, 0, ADDR_AUTOCONF);

    dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &s_global_ipaddr);
    if( dag != NULL )
    {
        rpl_set_prefix(dag, &s_prefix64, 64);
        PRINTF("Created a new RPL dag with ID: ");
        PRINT6ADDR(&dag->dag_id);
        PRINTF("\r\n");
    }
}
/******************************************************************************/
bool prefix64_is_set(void)
{
    return s_prefix64_is_set;
}
/******************************************************************************/
uip_ipaddr_t *get_global_address(void)
{
    return &s_global_ipaddr;
}
/******************************************************************************/
void request_prefix(void)
{
    //
}
/******************************************************************************/
static void print_local_addresses(void)
{
    int i;
    uint8_t state;

    PRINTF("Server IPv6 addresses:\r\n");
    for( i = 0; i < UIP_DS6_ADDR_NB; i++ )
    {
        state = uip_ds6_if.addr_list[i].state;
        if( uip_ds6_if.addr_list[i].isused
                && ( state == ADDR_TENTATIVE || state == ADDR_PREFERRED ) )
        {
            PRINTF("  ");
            PRINT6ADDR( &uip_ds6_if.addr_list[i].ipaddr);
            PRINTF("\r\n");
        }
    }
}
/******************************************************************************/
PROCESS_THREAD(border_router_process, ev, data)
{
    static struct etimer et;

    PROCESS_BEGIN();

    /* While waiting for the s_prefix64 to be sent through the SLIP connection, the future
     * border router can join an existing DAG as a parent or child, or acquire a default
     * router that will later take precedence over the SLIP fallback interface.
     * Prevent that by turning the radio off until we are initialized as a DAG root.
     */
    s_prefix64_is_set = 0;
    NETSTACK_MAC.off(0);

    PROCESS_PAUSE();

#if 0
    SENSORS_ACTIVATE(button_sensor);
#endif

    PRINTF("RPL-Border router started\r\n");
#if 0
    /* The border router runs with a 100% duty cycle in order to ensure high
     * packet reception rates.
     * Note if the MAC RDC is not turned off now, aggressive power management of the
     * cpu will interfere with establishing the SLIP connection
     */
    NETSTACK_MAC.off(1);
#endif



    /* manually set s_prefix64 here
     */
    uip_ipaddr_t prefix64;

    uip_ip6addr(&prefix64, PREFIX_ADDR0, PREFIX_ADDR1, PREFIX_ADDR2, PREFIX_ADDR3, PREFIX_ADDR4, PREFIX_ADDR5, PREFIX_ADDR6, PREFIX_ADDR7);
    set_prefix_64(&prefix64);

    /* Request s_prefix64 until it has been received */
    PRINTF("%s() - Request prefix until it has been received\r\n", __FUNCTION__);
    while(!s_prefix64_is_set)
    {
        etimer_set(&et, CLOCK_SECOND);
        request_prefix();
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }
    PRINTF("\r\n%s() - got prefix\r\n", __FUNCTION__);

    /* Now turn the radio on, but disable radio duty cycling.
     * Since we are the DAG root, reception delays would constrain mesh throughbut.
     */
    NETSTACK_MAC.off(1);

#if DEBUG || 1
    print_local_addresses();
#endif

    while(1)
    {
        PROCESS_YIELD();
#if 0
        if (ev == sensors_event && data == &button_sensor)
        {
            PRINTF("Initiating global repair\r\n");
            rpl_repair_root(RPL_DEFAULT_INSTANCE);
        }
#endif
    }

    PROCESS_END();
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/
