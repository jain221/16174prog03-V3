/**
 * @file  sensor_node.h
 * @brief A struct to maintain info about a remote sensor node.
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */

#ifndef SOURCE_INC_DATABUFFERS_SENSOR_NODE_H_
#define SOURCE_INC_DATABUFFERS_SENSOR_NODE_H_




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include <stdbool.h>

#include "net/ip/uip.h"
#include "sensor_data_list.h"




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
    bool            is_used;                /* is this element in use (used by sensor_node_list) */
    struct {
        uint8_t is_stale : 1;
        uint8_t for_deleting : 1;
        uint8_t is_dirty : 1;               /**< @brief Indicates if object has changed */
    } flags;
    uip_ipaddr_t    ipaddr;                 /* The IP address of the node */
    clock_time_t    last_msg_rx_time;       /* time of last message reception */
    SensorDataList  data_list;              /* The data-stream */
    uint16_t        id16;                   /* Transaction ID for communications between node and gateway  */
    uint8_t         wrong_id16_count;       /* Counter used to detect change in id16 */
    uint32_t        front_seq32;            /* The sequence-id of the element in the front of the queue */
    uint32_t        end_seq32;              /* The sequence-id of the element in the end of the queue */
    uint32_t        num_rx_packets;         /* Number of received packets */
    uint8_t         fw_version[3];          /* Firmware version major.minor.release */
    uint8_t         stratum;                /* SSTP stratum */
    double          lat;                    /* GPS */
    double          lon;                    /* GPS */
    uint32_t        last_long_msg_s;
    uint32_t        num_samples_waiting;    /* Number of samples waiting to be sent */
    uint16_t        bulb_current_ma_rms;    /* Bulb current (in mA RMS) */
    struct {
        uint32_t    num_samples_waiting;    /* Number of samples waiting to be sent */
    } shadow;
} SensorNode;




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


/** @brief Initialise the object */
void SensorNode_init(SensorNode *p_self);
bool SensorNode_destroy(SensorNode *p_self);

bool SensorNode_reset_data_stream(SensorNode *p_self, uint16_t id16, uint32_t seq32);

bool SensorNode_add_data(SensorNode *p_self, struct SensorData *p_sensor_data);
struct SensorData* SensorNode_remove_data(SensorNode *p_self);

uint32_t SensorNode_get_data_size(SensorNode *p_self);
uint32_t SensorNode_max_pop_len(SensorNode const *p_self, uint32_t limit);
uint32_t SensorNode_received_to_seq32(SensorNode const *p_self);

char const* SensorNode_get_status_string(SensorNode const *p_self);

void     SensorNode_mark_as_dirty(SensorNode *p_self);
void     SensorNode_clear_is_dirty(SensorNode *p_self);
bool     SensorNode_is_dirty(SensorNode const *p_self);

void     SensorNode_mark_as_stale(SensorNode *p_self);
void     SensorNode_update_last_msg_rx_time(SensorNode *p_self);
uint32_t SensorNode_seconds_since_last_msg_rx(SensorNode const *p_self);


void SensorNode_mark_for_deletion(SensorNode *p_self);


#ifdef __cplusplus
}
#endif




/*******************************************************************************
*                               CONFIGURATION ERRORS
*******************************************************************************/




#endif /* SOURCE_INC_DATABUFFERS_SENSOR_NODE_H_ */
