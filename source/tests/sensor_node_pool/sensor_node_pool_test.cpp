/**
 * @file  sensor_node_pool_test.cpp
 * @brief Unit-tests for the sensor-node-pool module
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include "sensor_node_pool.h"
#include "sensor_node_list.h"

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"




/*******************************************************************************
*                                  Test Group
*******************************************************************************/
TEST_GROUP( test_sensor_node_pool )
{
    /**************************************************************************/
    TEST_SETUP()
    {
        // setup() is run before each test
        SNL_init();
    }
    /**************************************************************************/
    TEST_TEARDOWN()
    {
        // teardown() is run after each test
        mock().clear();
    }
    /**************************************************************************/
    /** @brief A utility function to initialise an IP6 address
     */
    void initialise_ipaddr__(uip_ipaddr_t &ipaddr, uint16_t seq16)
    {
        // Initialise structure to 0
        memset(&ipaddr, 0, sizeof(uip_ipaddr_t));

        // Set lower 16-bits of IP6 address
        ipaddr.u16[0] = seq16;
    }
    /**************************************************************************/
    void create_nodes__(std::vector<SensorNode*> &nodes, uint32_t count)
    {
        uip_ipaddr_t ipaddr;

        for(uint32_t ii=0U; ii<count; ++ii)
        {
            bool was_created;

            initialise_ipaddr__(ipaddr, (uint16_t) ii);

            SensorNode *p_sensor_node = SNL_find(&ipaddr, true, &was_created);

            CHECK( p_sensor_node != nullptr );
            CHECK( p_sensor_node->is_used );
            CHECK( p_sensor_node->flags.is_dirty );
            CHECK( was_created );

            nodes.push_back(p_sensor_node);
        }
    }
    /**************************************************************************/
};
/******************************************************************************/
TEST( test_sensor_node_pool, determine_which_node_should_send_data__when__empty_list )
{
    SensorNode *p_sensor_node=nullptr;

    POINTERS_EQUAL(nullptr, determine_which_node_should_send_data(p_sensor_node) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_pool, determine_which_node_should_send_data__when__one_node__but_no_data )
{
    std::vector<SensorNode*> nodes;

    /* Create two nodes */
    create_nodes__(nodes, 1);


    /* Run test */
    SensorNode *p_sensor_node=nullptr;

    POINTERS_EQUAL(nullptr, determine_which_node_should_send_data(p_sensor_node) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_pool, determine_which_node_should_send_data__when__random_data )
{
    std::vector<SensorNode*> nodes;

    /* Create two nodes */
    create_nodes__(nodes, 4);
    nodes[0]->num_samples_waiting = 1200U;
    nodes[2]->num_samples_waiting = 1200U;
    nodes[3]->num_samples_waiting = 1200U;


    /* Run test */
    SensorNode *p_iter=nullptr;

    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[0], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[0], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[0], p_iter);

    nodes[0]->num_samples_waiting = 0U;

    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[2], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[2], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[2], p_iter);

    nodes[2]->num_samples_waiting = 0U;

    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[3], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[3], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[3], p_iter);

    nodes[3]->num_samples_waiting = 0U;

    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nullptr, p_iter);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_pool, determine_which_node_should_send_data__when__random_data2 )
{
    std::vector<SensorNode*> nodes;

    /* Create two nodes */
    create_nodes__(nodes, 6);
    nodes[2]->num_samples_waiting = 1200U;
    nodes[4]->num_samples_waiting = 1200U;
    nodes[5]->num_samples_waiting = 1200U;


    /* Run test */
    SensorNode *p_iter=nullptr;

    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[2], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[2], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[2], p_iter);

    nodes[2]->num_samples_waiting = 0U;
    nodes[0]->num_samples_waiting = 1200U;

    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[4], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[4], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[4], p_iter);

    nodes[4]->num_samples_waiting = 0U;

    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[5], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[5], p_iter);
    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[5], p_iter);

    nodes[5]->num_samples_waiting = 0U;

    p_iter = determine_which_node_should_send_data(p_iter);
    POINTERS_EQUAL(nodes[0], p_iter);

    mock().checkExpectations();
}
/******************************************************************************/
