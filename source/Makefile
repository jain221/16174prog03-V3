CONTIKI_PROJECT = 16174prog03


all: $(CONTIKI_PROJECT)


V=1
#SMALL=1


CONTIKI     = ../../contiki
CONTIKI_ALC = ../../contiki-alc
APPDIRS     += ./apps


CONTIKI_WITH_IPV6 = 1
CONTIKI_WITH_RPL  = 1

ALC_POLE_CLUSTER_CTRL_ROOT_NODE = 1
ALC_POLE_DATA_LINK_ROOT_NODE = 1
ALC_USING_STM32_BLUENRG_BLE = 1

APPS += alc_blink_app
APPS += alc_bluetooth_server
APPS += alc_pole_cluster_ctrl
APPS += alc_pole_data_link
APPS += alc_shell
APPS += serial-shell
APPS += servreg-hack


#CFLAGS += -DTRACE=1
CFLAGS += -DDBG_PRINTF_USE_PRINTF=1
CFLAGS += -DPROJECT_CONF_H="\"project-conf.h\""
CFLAGS += -DGATEWAY_BUILD=1
CFLAGS += -DPRODUCTION_RELEASE=1


# The following is to allow the GPS library to work on our system
CFLAGS += -Dtimegm=mktime


INCDIRS += \
		inc \
		inc/databuffers \
		inc/gps \
		inc/modem \
		inc/net \
		inc/shell \
		inc/storage \
		$(CONTIKI_ALC)/inc/timer_obj


PROJECTDIRS += \
		src \
		src/databuffers \
		src/gps \
		src/modem \
		src/net \
		src/shell \
		src/storage \
		$(CONTIKI_ALC)/src/timer_obj \
		$(CONTIKI_ALC)/third_party/minmea


PROJECT_SOURCEFILES += \
		gps_data.c \
		gps_time_ctrl.c \
		ntp_time_ctrl.c \
		server_time_ctrl.c \
		rpl_border_router.c \
		uip_log.c


# src/databuffers folder
PROJECT_SOURCEFILES += \
		sensor_data_list.c \
		sensor_data_pool.c \
		sensor_node_list.c \
		sensor_node.c \
		sensor_node_pool.c


# src/modem folder
PROJECT_SOURCEFILES += \
		modem_ctrl.c \
		modem_drv_sim808.c


# src/net folder
PROJECT_SOURCEFILES += \
		data_upload_client.c \
		data_upload_msg.c


# src/shell folder
PROJECT_SOURCEFILES += \
		16174prog03_shell_factory.c \
		16174prog03_shell_nodes.c


# src/storage folder
PROJECT_SOURCEFILES += \
		nv_settings.c


# Files in contiki-alc/src folder
PROJECT_SOURCEFILES += \
		alc_eat_string_tokens.c \
		alc_ipaddr_snprintf.c \
		alc_logger.c \
		alc_math.c \
		alc_nmea_utils.c \
		alc_radio_setup.c \
		alc_shell_utils.c \
		alc_store_string_utils.c \
		alc_string.c \
		alc_timer_counter.c \
		alc_timer_obj.c


# Files in contiki-alc/third_party folder
xxxxPROJECT_SOURCEFILES += \
		minmea.c


ifeq ($(ALC_USING_STM32_BLUENRG_BLE),1)
# Add files to support STM32 Bluetooth library
STM32_BlueNRG = ../../STM32_BlueNRG

INCDIRS += \
		inc/bt \
		$(CONTIKI_ALC)/inc/bt \
		$(STM32_BlueNRG)/SimpleBlueNRG_HCI/includes

PROJECTDIRS += \
		src/bt \
		$(CONTIKI_ALC)/src/bt \
		$(STM32_BlueNRG)/SimpleBlueNRG_HCI/hci \
		$(STM32_BlueNRG)/SimpleBlueNRG_HCI/hci/controller \
		$(STM32_BlueNRG)/SimpleBlueNRG_HCI/utils

PROJECT_SOURCEFILES += \
		external_ble_interface.c \
		bt_is_downloading.c \
		sensor_service.c \
		hci.c \
		ble_list.c \
		gp_timer.c \
		osal.c \
		bluenrg_gap_aci.c \
		bluenrg_gatt_aci.c \
		bluenrg_hal_aci.c \
		bluenrg_utils_small.c
endif


include $(CONTIKI_ALC)/Makefile.include
