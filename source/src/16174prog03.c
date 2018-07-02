/**
 * @file  16174prog03.c
 * @brief 16174prog03 program
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#include "contiki.h"

#include "16174prog03_shell_factory.h"
#include "16174prog03_shell_nodes.h"
#include "alc_blink_app.h"
#if ALC_USING_STM32_BLUENRG_BLE
#include "alc_bluetooth_server.h"
#endif
#include "alc_shell_eeprom.h"
#include "alc_shell_modem.h"
#include "alc_shell_network.h"
#include "alc_shell_radio.h"
#include "alc_shell_random.h"
#include "alc_shell_rtcc.h"
#include "alc_shell_rtimer.h"
#include "alc_shell_status.h"
#include "alc_shell_time.h"
#include "serial-shell.h"
#include "shell.h"


PROCESS_NAME(alc_pole_cluster_ctrl_server_process);
PROCESS_NAME(alc_pole_data_link_root_process);
PROCESS_NAME(border_router_process);


PROCESS(start_shell, "start shell");


/******************************************************************************/
/*
 * List of processes that will be auto-started in the main() function.
 */
AUTOSTART_PROCESSES(
        &alc_blink_app_process,
#if ALC_USING_STM32_BLUENRG_BLE
        &alc_bluetooth_server_process,
#endif
        &alc_pole_cluster_ctrl_server_process,
        &alc_pole_data_link_root_process,
        &border_router_process,
        &start_shell
);
/******************************************************************************/
/* This process just starts the shell running, and then exit's
 */
PROCESS_THREAD(start_shell, ev, data)
{
    PROCESS_BEGIN();

    serial_shell_init();
    alc_shell_eeprom_init();
    alc_shell_modem_init();
    alc_shell_network_init();
    alc_shell_radio_init();
    alc_shell_random_init();
    alc_shell_rtcc_init();
    alc_shell_rtimer_init();
    alc_shell_status_init();
    alc_shell_time_init();
    C16174prog03_shell_factory_init();
    C16174prog03_shell_nodes_init();
    shell_reboot_init();

    PROCESS_END();
}
/******************************************************************************/
