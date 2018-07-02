/**
 * @file  sensor_node_test.cpp
 * @brief Unit-tests for the sensor-node module
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include <algorithm>
#include <iostream>
#include <memory.h>
#include <vector>

#include "sensor_node.h"

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"






/*******************************************************************************
*                                  Test Group
*******************************************************************************/
TEST_GROUP( test_sensor_node )
{
    /**************************************************************************/
    TEST_SETUP()
    {
        // setup() is run before each test
    }
    /**************************************************************************/
    TEST_TEARDOWN()
    {
        // teardown() is run after each test
        mock().clear();
    }
    /**************************************************************************/
};
/******************************************************************************/
TEST( test_sensor_node, init )
{
    SensorNode sensor_node;

    SensorNode_init(&sensor_node);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node, init_with_null_pointer )
{
    SensorNode_init(nullptr);

    mock().checkExpectations();
}
/******************************************************************************/




/*******************************************************************************
*                            Test Group max-pop-length
*******************************************************************************/
TEST_GROUP( test_sensor_node__max_pop_len )
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
TEST( test_sensor_node__max_pop_len, empty_list )
{
    std::vector<uint32_t> values;

    UNSIGNED_LONGS_EQUAL(0U, test_max_pop_length__(0, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node__max_pop_len, len1 )
{
    std::vector<uint32_t> values = {1};

    UNSIGNED_LONGS_EQUAL(1U, test_max_pop_length__(1, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node__max_pop_len, len1__missing_front )
{
    std::vector<uint32_t> values = {1};

    UNSIGNED_LONGS_EQUAL(0U, test_max_pop_length__(0, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node__max_pop_len, len10 )
{
    std::vector<uint32_t> values = {1,2,3,4,5,6,7,8,9,10};

    UNSIGNED_LONGS_EQUAL(10U, test_max_pop_length__(1, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node__max_pop_len, len10__missing_front )
{
    std::vector<uint32_t> values = {1,2,3,4,5,6,7,8,9,10};

    UNSIGNED_LONGS_EQUAL(10U, test_max_pop_length__(1, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node__max_pop_len, len10__broken_seq )
{
    std::vector<uint32_t> values = {1,2,3,4,5,7,8,9,10,11};

    UNSIGNED_LONGS_EQUAL(5U, test_max_pop_length__(1, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_node__max_pop_len, len10__broken_seq2 )
{
    std::vector<uint32_t> values = {(uint32_t) -3,(uint32_t) -2,(uint32_t) -1,0,1,2,3,4,5,6,7,8,9,10};

    UNSIGNED_LONGS_EQUAL(14U, test_max_pop_length__(-3, values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
