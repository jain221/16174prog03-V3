/**
 * @file  sensor_data_list__max_pop_len_test.cpp
 * @brief Unit-tests for the max-pop-length function
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

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "sensor_data_list.h"


#define DEBUG 0


/*******************************************************************************
*                                  Test Group
*******************************************************************************/
TEST_GROUP( test_sensor_data_list__max_pop_len )
{
    SensorDataList list1;
    /**************************************************************************/
    TEST_SETUP()
    {
        // setup() is run before each test
        SensorDataList_init(&list1);
    }
    /**************************************************************************/
    TEST_TEARDOWN()
    {
        // teardown() is run after each test
        mock().clear();
    }
    /**************************************************************************/
    void print_vector__(std::vector<uint32_t> const &source)
    {
#if DEBUG
        std::cout << std::endl << "Vector = ";
        for(std::vector<uint32_t>::size_type ii = 0; ii != source.size(); ++ii)
        {
            std::cout << source[ii] << " ";
        }
#endif
    }
    /**************************************************************************/
    void print_vector__(SensorDataList const &source)
    {
#if DEBUG
        std::cout << std::endl << "Vector = ";
        for(auto iter = source.p_first; iter!=NULL; iter = iter->p_next)
        {
            std::cout << iter->seq32 << " ";
        }
#endif
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
    uint32_t test_max_pop_length__(std::vector<uint32_t> const &source, uint32_t limit)
    {
        SensorDataList_init(&list1);

        print_vector__(source);

        std::vector<struct SensorData> data;

        data.reserve(source.size());

        for(std::vector<uint32_t>::size_type ii = 0; ii != source.size(); ++ii)
        {
            initialise_sensor_data__(data[ii], source[ii]);

            CHECK_TRUE( SensorDataList_insert(&list1, &data[ii]) );
        }

        return SensorDataList_max_pop_len(&list1, limit);
    }
    /**************************************************************************/
};
/******************************************************************************/




/*******************************************************************************
*                                    Tests
*******************************************************************************/
TEST( test_sensor_data_list__max_pop_len, init )
{
    UNSIGNED_LONGS_EQUAL(0, SensorDataList_max_pop_len(&list1, 99));

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__max_pop_len, empty_list )
{
    std::vector<uint32_t> values;

    UNSIGNED_LONGS_EQUAL(0U, test_max_pop_length__(values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__max_pop_len, len1 )
{
    std::vector<uint32_t> values = {1};

    UNSIGNED_LONGS_EQUAL(1U, test_max_pop_length__(values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__max_pop_len, len2 )
{
    std::vector<uint32_t> values = {1,2};

    UNSIGNED_LONGS_EQUAL(2U, test_max_pop_length__(values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__max_pop_len, len10 )
{
    std::vector<uint32_t> values = {11,12,13,14,15,16,17,18,19,20};

    UNSIGNED_LONGS_EQUAL(10U, test_max_pop_length__(values, 99) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__max_pop_len, gap_len1 )
{
    std::vector<uint32_t> values = {1,3,4,5,6};

    UNSIGNED_LONGS_EQUAL(1U, test_max_pop_length__(values, 99) );
    UNSIGNED_LONGS_EQUAL(1U, test_max_pop_length__(values, 1) );
    UNSIGNED_LONGS_EQUAL(0U, test_max_pop_length__(values, 0) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__max_pop_len, gap_len2 )
{
    std::vector<uint32_t> values = {1,2,4,5,6,7};

    UNSIGNED_LONGS_EQUAL(2U, test_max_pop_length__(values, 99) );
    UNSIGNED_LONGS_EQUAL(1U, test_max_pop_length__(values, 1) );
    UNSIGNED_LONGS_EQUAL(0U, test_max_pop_length__(values, 0) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__max_pop_len, gap_len10 )
{
    std::vector<uint32_t> values = {11,12,13,14,15,16,17,18,19,20,22,23,24,25};

    UNSIGNED_LONGS_EQUAL(10U, test_max_pop_length__(values, 99) );
    UNSIGNED_LONGS_EQUAL(5U,  test_max_pop_length__(values, 5) );
    UNSIGNED_LONGS_EQUAL(0U,  test_max_pop_length__(values, 0) );

    mock().checkExpectations();
}
/******************************************************************************/
