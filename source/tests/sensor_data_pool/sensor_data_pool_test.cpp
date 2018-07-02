/**
 * @file  sensor_data_pool_test.cpp
 * @brief Unit-tests for the data-pool module
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "sensor_data_pool.h"




/*******************************************************************************
*                                  Test Group
*******************************************************************************/
TEST_GROUP( test_sensor_data_pool )
{
    /**************************************************************************/
    TEST_SETUP()
    {
        // setup() is run before each test
        SensorDataPool_init();
    }
    /**************************************************************************/
    TEST_TEARDOWN()
    {
        // teardown() is run after each test
        SensorDataPool_check_links();
       // mock().clear();
    }
    /**************************************************************************/
};
/******************************************************************************/




/*******************************************************************************
*                                    Tests
*******************************************************************************/
TEST( test_sensor_data_pool, init )
{
    LONGS_EQUAL(10, SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(10, SensorDataPool_get_min_size() );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_pool, test_get )
{
    auto p_obj = SensorDataPool_get();

    POINTERS_EQUAL(nullptr, p_obj->p_next);

    LONGS_EQUAL(9,  SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(9,  SensorDataPool_get_min_size() );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_pool, test_get_2 )
{
    auto p_obj = SensorDataPool_get();
    POINTERS_EQUAL(nullptr, p_obj->p_next);

    p_obj = SensorDataPool_get();
    POINTERS_EQUAL(nullptr, p_obj->p_next);

    LONGS_EQUAL(8,  SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(8,  SensorDataPool_get_min_size() );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_pool, test_get_and_return )
{
    auto p_obj = SensorDataPool_get();

    POINTERS_EQUAL(nullptr, p_obj->p_next);

    SensorDataPool_return(p_obj);

    LONGS_EQUAL(10,  SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(9,  SensorDataPool_get_min_size() );
mock().expectOneCall("SensorDataPool_return").withOutputParameterReturning("p_data", &LONGS_EQUAL);
    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_pool, test_get_empty )
{
    auto p_obj = SensorDataPool_get();

    p_obj = SensorDataPool_get();
    p_obj = SensorDataPool_get();
    p_obj = SensorDataPool_get();
    p_obj = SensorDataPool_get();
    p_obj = SensorDataPool_get();
    p_obj = SensorDataPool_get();
    p_obj = SensorDataPool_get();
    p_obj = SensorDataPool_get();

    mock().expectOneCall("AlcLogger_log_warning");
    p_obj = SensorDataPool_get();

    p_obj = SensorDataPool_get();
    POINTERS_EQUAL(nullptr, p_obj);

    LONGS_EQUAL(0,  SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(0,  SensorDataPool_get_min_size() );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_pool, test_min_size_goes_down_as_objects_pulled )
{
    struct SensorData *obj_list[6];

    LONGS_EQUAL(10, SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(10, SensorDataPool_get_min_size() );

    for(uint32_t ii=0U; ii<6; ii++)
    {
        obj_list[ii] = SensorDataPool_get();

        LONGS_EQUAL(9-ii,  SensorDataPool_get_size() );
        LONGS_EQUAL(10,    SensorDataPool_get_max_size() );
        LONGS_EQUAL(9-ii,  SensorDataPool_get_min_size() );
    }

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_sensor_data_pool, test_reset_min_size )
{
    struct SensorData *obj_list[6];

    for(uint32_t ii=0U; ii<6; ii++)
    {
        obj_list[ii] = SensorDataPool_get();
    }

    LONGS_EQUAL(4,  SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(4,  SensorDataPool_get_min_size() );


    for(uint32_t ii=0U; ii<3; ii++)
    {
        SensorDataPool_return(obj_list[ii]);

        LONGS_EQUAL(5+ii, SensorDataPool_get_size() );
        LONGS_EQUAL(10,   SensorDataPool_get_max_size() );
        LONGS_EQUAL(4,    SensorDataPool_get_min_size() );
    }

    LONGS_EQUAL(7,  SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(4,  SensorDataPool_get_min_size() );

    SensorDataPool_reset_min_size();

    LONGS_EQUAL(7,  SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(7,  SensorDataPool_get_min_size() );

    for(uint32_t ii=3U; ii<6; ii++)
    {
        SensorDataPool_return(obj_list[ii]);

        LONGS_EQUAL(5+ii, SensorDataPool_get_size() );
        LONGS_EQUAL(10,   SensorDataPool_get_max_size() );
        LONGS_EQUAL(7,    SensorDataPool_get_min_size() );
    }

    LONGS_EQUAL(10,  SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(7,  SensorDataPool_get_min_size() );

    SensorDataPool_reset_min_size();

    LONGS_EQUAL(10, SensorDataPool_get_size() );
    LONGS_EQUAL(10, SensorDataPool_get_max_size() );
    LONGS_EQUAL(10, SensorDataPool_get_min_size() );

    mock().checkExpectations();
}
/******************************************************************************/
