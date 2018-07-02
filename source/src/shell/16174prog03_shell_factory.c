/**
 * @file  16174prog03_shell_factory.c
 * @brief Factory shell commands.
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "16174prog03_shell_factory.h"

#include <stdio.h>
#include <string.h>

#include "contiki.h"

#include "dev/eeprom.h"
#include "dev/watchdog.h"
#include "nv_settings.h"
#include "shell.h"




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




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
PROCESS(C16174prog03_shell_factory_process, "factory");
SHELL_COMMAND(factory_command,
          "factory",
          "factory: print factory debug info",
          &C16174prog03_shell_factory_process);
/******************************************************************************/
PROCESS_THREAD(C16174prog03_shell_factory_process, ev, data)
{
    static const uint8_t s_ip4_addr[4]={127U,0U,0U,1U};
    static const uint16_t s_portnum=0xFFFFU;
    static const uint8_t s_unknown[10]={9U,'u','n','d','e','f','i','n','e','d'};
    static struct etimer etimer;
    static const unsigned char buff[16] = {
            0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
            0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU
    };

    PROCESS_BEGIN();

    if( strcmp(data, "reset") == 0 )
    {
        static eeprom_addr_t addr;

        printf("factory reset\r\n");

        /* erase EEPROM storage */
        for(addr=0U; addr<256U; addr+=16U)
        {
            eeprom_write( addr, buff, sizeof(buff));
            etimer_set(&etimer, (CLOCK_SECOND / 5) );
            PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
        }


        Store_write_cloud_ipv4(s_ip4_addr);
        etimer_set(&etimer, (CLOCK_SECOND / 5) );
        PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
        Store_write_cloud_portnum(&s_portnum);
        etimer_set(&etimer, (CLOCK_SECOND / 5) );
        PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
        Store_write_modem_apn(s_unknown);
        etimer_set(&etimer, (CLOCK_SECOND / 5) );
        PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
        Store_write_modem_username(s_unknown);
        etimer_set(&etimer, (CLOCK_SECOND / 5) );
        PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
        Store_write_modem_password(s_unknown);
        etimer_set(&etimer, (CLOCK_SECOND / 5) );
        PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
        Store_write_pan_ch(11U);
        etimer_set(&etimer, (CLOCK_SECOND / 5) );
        PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
        Store_write_pan_id(0xABCDU);


        /* reboot */
        shell_output_str(&factory_command,
                 "Rebooting the node in four seconds...", "");

        etimer_set(&etimer, (4U * (CLOCK_SECOND) ) );
        PROCESS_WAIT_UNTIL(etimer_expired(&etimer));

        watchdog_reboot();
    }

    PROCESS_END();
}
/******************************************************************************/
void C16174prog03_shell_factory_init(void)
{
    shell_register_command(&factory_command);
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/
