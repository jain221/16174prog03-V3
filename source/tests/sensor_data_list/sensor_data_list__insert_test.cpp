/**
 * @file  sensor_data_list__insert_test.cpp
 * @brief Unit-tests for the insert function
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
TEST_GROUP( test_sensor_data_list__insert )
{
    SensorDataList list1;
    std::vector<struct SensorData> data1;
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
    void print_vector__(std::vector<uint32_t> &source)
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
    void print_vector__(SensorDataList &source)
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
    bool test_insert__(std::vector<uint32_t> &source)
    {
        print_vector__(source);

        data1.reserve(source.size());

        for(std::vector<uint32_t>::size_type ii = 0; ii != source.size(); ++ii)
        {
            initialise_sensor_data__(data1[ii], source[ii]);

            CHECK_TRUE( SensorDataList_insert(&list1, &data1[ii]) );
        }

        print_vector__(list1);

        // Sorting the int vector
        sort(source.begin(), source.end());
        print_vector__(source);


#if DEBUG
        std::cout << std::endl << "Vector = ";
#endif
        std::vector<int>::size_type count = 0;
        for(auto iter = list1.p_first; iter!= NULL; iter = iter->p_next)
        {
#if DEBUG
            std::cout << iter->seq32 << " ";
#endif
            UNSIGNED_LONGS_EQUAL(source[count], iter->seq32);
            count++;
        }

        LONGS_EQUAL(source.size(), count);

        SensorDataList_check_links(&list1);

        return true;
    }
    /**************************************************************************/
};
/******************************************************************************/




/*******************************************************************************
*                                    Tests
*******************************************************************************/
TEST( test_sensor_data_list__insert, init )
{
    LONGS_EQUAL(0, SensorDataList_get_size(&list1) );
    CHECK_TRUE( SensorDataList_is_empty(&list1) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert1 )
{
    struct SensorData data;
    memset(&data, 0, sizeof(data));

    CHECK_TRUE( SensorDataList_insert(&list1, &data) );

    LONGS_EQUAL(1, SensorDataList_get_size(&list1) );
    CHECK_FALSE( SensorDataList_is_empty(&list1) );

    SensorDataList_check_links(&list1);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert2 )
{
    struct SensorData data[2];
    memset(data, 0, sizeof(data));

    data[0].seq32 = 0;
    data[1].seq32 = 1;

    CHECK_TRUE( SensorDataList_insert(&list1, &data[0]) );
    CHECK_TRUE( SensorDataList_insert(&list1, &data[1]) );

    LONGS_EQUAL(2, SensorDataList_get_size(&list1) );
    CHECK_FALSE( SensorDataList_is_empty(&list1) );

    SensorDataList_check_links(&list1);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert3 )
{
    struct SensorData data[2];
    memset(data, 0, sizeof(data));

    data[0].seq32 = 1;
    data[1].seq32 = 0;

    CHECK_TRUE( SensorDataList_insert(&list1, &data[0]) );
    CHECK_TRUE( SensorDataList_insert(&list1, &data[1]) );

    LONGS_EQUAL(2, SensorDataList_get_size(&list1) );
    CHECK_FALSE( SensorDataList_is_empty(&list1) );

    SensorDataList_check_links(&list1);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert_sequence )
{
    std::vector<uint32_t> values = {1,2,3,4,5,6};

    CHECK_TRUE( test_insert__(values) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert_sequence_with_gaps )
{
    std::vector<uint32_t> values = {1,2,3,5,6};

    CHECK_TRUE( test_insert__(values) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert_reverse_sequence )
{
    std::vector<uint32_t> values = {10,9,8,7,6,5,4,3,2,1,0};

    CHECK_TRUE( test_insert__(values) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert_reverse_sequence_with_gaps )
{
    std::vector<uint32_t> values = {10,9,8,7,4,3,2,1,0};

    CHECK_TRUE( test_insert__(values) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert_jumbled_5 )
{
    std::vector<uint32_t> values = {1,2,3,4,6,7,8,9,5};

    CHECK_TRUE( test_insert__(values) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert_jumbled_6 )
{
    std::vector<uint32_t> values = {4,5,1,2,8,9,10,3,6,7,11,12};

    CHECK_TRUE( test_insert__(values) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert_jumbled_7 )
{
    std::vector<uint32_t> values = {23,76,24,6,9,12,56,8,798};

    CHECK_TRUE( test_insert__(values) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_list__insert, insert_fail )
{
    std::vector<uint32_t> values = {1,2,3,4,5,6};

    CHECK_TRUE( test_insert__(values) );


    struct SensorData data[3];

    initialise_sensor_data__(data[0], 1);
    initialise_sensor_data__(data[1], 3);
    initialise_sensor_data__(data[2], 6);

    CHECK_FALSE( SensorDataList_insert(&list1, &data[0]) );
    CHECK_FALSE( SensorDataList_insert(&list1, &data[1]) );
    CHECK_FALSE( SensorDataList_insert(&list1, &data[2]) );

    mock().checkExpectations();
}
/******************************************************************************/
