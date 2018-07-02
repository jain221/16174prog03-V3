/**
 * @file  sensor_node_list_test.cpp
 * @brief Unit-tests for the sensor-node-list module
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

#include "sensor_node_list.h"

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"




/*******************************************************************************
*                                  Test Group
*******************************************************************************/
TEST_GROUP( test_sensor_node_list )
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
TEST( test_sensor_node_list, init )
{
    UNSIGNED_LONGS_EQUAL(0U, SNL_get_size() );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, create_new_node_with_find )
{
    bool was_created;
    uip_ipaddr_t ipaddr;

    SensorNode *p_sensor_node = SNL_find(&ipaddr, true, &was_created);

    CHECK( p_sensor_node != nullptr );
    CHECK( p_sensor_node->is_used );
    CHECK( p_sensor_node->flags.is_dirty );
    CHECK( was_created );
    UNSIGNED_LONGS_EQUAL(1U, SNL_get_size() );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, create_two_new_nodes_with_find )
{
    bool was_created;
    uip_ipaddr_t ipaddr;

    /* Create 1st node */
    ipaddr.u8[0] = 0U;
    SensorNode *p_sensor_node1 = SNL_find(&ipaddr, true, &was_created);

    CHECK( p_sensor_node1 != nullptr );
    CHECK( p_sensor_node1->is_used );
    CHECK( p_sensor_node1->flags.is_dirty );
    CHECK( was_created );
    UNSIGNED_LONGS_EQUAL(1U, SNL_get_size() );


    /* Create 2nd node */
    ipaddr.u8[0] = 1U;
    SensorNode *p_sensor_node2 = SNL_find(&ipaddr, true, &was_created);

    CHECK( p_sensor_node2 != nullptr );
    CHECK( p_sensor_node2->is_used );
    CHECK( p_sensor_node2->flags.is_dirty );
    CHECK( was_created );
    UNSIGNED_LONGS_EQUAL(2U, SNL_get_size() );

    CHECK( p_sensor_node1 != p_sensor_node2 );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, is_in_list )
{
    bool was_created;
    uip_ipaddr_t ipaddr;

    CHECK_FALSE( SNL_is_in_list(nullptr) );

    SensorNode *p_sensor_node = SNL_find(&ipaddr, true, &was_created);

    CHECK( p_sensor_node != nullptr );
    CHECK( SNL_is_in_list(p_sensor_node) );

    CHECK_FALSE( SNL_is_in_list(nullptr) );

    mock().checkExpectations();
}
/*******************************************************************************
 *              Test SNL_find_first_active_node() function
 ******************************************************************************/
TEST( test_sensor_node_list, find_first__when__empty_list )
{
    POINTERS_EQUAL(nullptr, SNL_find_first_active_node() );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, find_first__when__one_node_in_list )
{
    std::vector<SensorNode*> nodes;

    create_nodes__(nodes, 1);

    POINTERS_EQUAL(nodes[0], SNL_find_first_active_node() );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, find_first__when__two_node_in_list )
{
    std::vector<SensorNode*> nodes;

    create_nodes__(nodes, 2);

    POINTERS_EQUAL(nodes[0], SNL_find_first_active_node() );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, find_first__when__one_node_in_list_not_first_element )
{
    std::vector<SensorNode*> nodes;

    create_nodes__(nodes, 2);
    nodes[0]->is_used = false;

    POINTERS_EQUAL(nodes[1], SNL_find_first_active_node() );

    mock().checkExpectations();
}
/*******************************************************************************
 *              Test SNL_find_next_active_node() function
 ******************************************************************************/
TEST( test_sensor_node_list, find_next_active_node__when__empty_list )
{
    SensorNode *p_sensor_node=nullptr;

    POINTERS_EQUAL(nullptr, SNL_find_next_active_node(p_sensor_node, false));

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, find_next_active_node__when__one_node )
{
    std::vector<SensorNode*> nodes;

    /* Create one node */
    create_nodes__(nodes, 1);


    /* Run test
     * Should get sequence... node0, node0, node0, etc
     */
    SensorNode *p_iter=nullptr;
    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, find_next_active_node__when__two_nodes )
{
    std::vector<SensorNode*> nodes;

    /* Create two nodes */
    create_nodes__(nodes, 2);


    /* Run test
     * Should get sequence... node0, node1, node0, node1, node0, etc
     */
    SensorNode *p_iter=nullptr;
    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[1], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[1], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, find_next_active_node__when__three_nodes )
{
    std::vector<SensorNode*> nodes;

    /* Create 3 nodes */
    create_nodes__(nodes, 3);


    /* Run test
     * Should get sequence... node0, node1, node2, node0, node1, etc
     */
    SensorNode *p_iter=nullptr;
    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[1], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[2], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[1], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[2], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[0], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[1], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[2], p_iter);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list, find_next_active_node__when__random_list )
{
    std::vector<SensorNode*> nodes;

    /* Create 8 nodes and then mark node 0 and node 4 as not active */
    create_nodes__(nodes, 8);
    nodes[0]->is_used = false;
    nodes[4]->is_used = false;


    /* Run test
     * Should get sequence... node1, node2, node3, node5, node6, node7, node1, etc
     */
    SensorNode *p_iter=nullptr;
    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[1], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[2], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[3], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[5], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[6], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[7], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[1], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[2], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[3], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[5], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[6], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[7], p_iter);

    p_iter = SNL_find_next_active_node(p_iter, true);
    POINTERS_EQUAL(nodes[1], p_iter);

    mock().checkExpectations();
}
/******************************************************************************/












#if 0

/*******************************************************************************
*                            Test Group max-pop-length
*******************************************************************************/
TEST_GROUP( test_sensor_node_list__max_pop_len )
{
    SensorNode sensor_node1;
    /**************************************************************************/
    TEST_SETUP()
    {
        // setup() is run before each test
        SensorNode_init(&sensor_node1);
    }
    /**************************************************************************/
    TEST_TEARDOWN()
    {
        // teardown() is run after each test
        mock().clear();
    }
    /**************************************************************************/
    void initialise_sensor_data__(struct SensorData &data, uint32_t seq32)
    {
        // Initialise structure to 0
        memset(&data, 0, sizeof(struct SensorData));

        // Set sequence number
        data.seq32 = seq32;
    }
    /**************************************************************************/
    uint32_t test_max_pop_length__(uint32_t front_seq32, std::vector<uint32_t> const &source, uint32_t limit)
    {
        sensor_node1.front_seq32 = front_seq32;

        std::vector<struct SensorData> data;

        data.reserve(source.size());

        for(std::vector<uint32_t>::size_type ii = 0; ii != source.size(); ++ii)
        {
            initialise_sensor_data__(data[ii], source[ii]);

            CHECK_TRUE( SensorDataList_insert(&sensor_node1.data_list, &data[ii]) );
        }

        return SensorNode_max_pop_len(&sensor_node1, limit);
    }
    /**************************************************************************/
};
/******************************************************************************/
TEST( test_sensor_node_list__max_pop_len, empty_list )
{
    std::vector<uint32_t> values;

    UNSIGNED_LONGS_EQUAL(0U, test_max_pop_length__(0, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list__max_pop_len, len1 )
{
    std::vector<uint32_t> values = {1};

    UNSIGNED_LONGS_EQUAL(1U, test_max_pop_length__(1, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list__max_pop_len, len1__missing_front )
{
    std::vector<uint32_t> values = {1};

    UNSIGNED_LONGS_EQUAL(0U, test_max_pop_length__(0, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list__max_pop_len, len10 )
{
    std::vector<uint32_t> values = {1,2,3,4,5,6,7,8,9,10};

    UNSIGNED_LONGS_EQUAL(10U, test_max_pop_length__(1, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list__max_pop_len, len10__missing_front )
{
    std::vector<uint32_t> values = {1,2,3,4,5,6,7,8,9,10};

    UNSIGNED_LONGS_EQUAL(10U, test_max_pop_length__(1, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list__max_pop_len, len10__broken_seq )
{
    std::vector<uint32_t> values = {1,2,3,4,5,7,8,9,10,11};

    UNSIGNED_LONGS_EQUAL(5U, test_max_pop_length__(1, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node_list__max_pop_len, len10__broken_seq2 )
{
    std::vector<uint32_t> values = {(uint32_t) -3,(uint32_t) -2,(uint32_t) -1,0,1,2,3,4,5,6,7,8,9,10};

    UNSIGNED_LONGS_EQUAL(14U, test_max_pop_length__(-3, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/


#endif
