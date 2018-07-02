/**
 * @file  data_upload_msg_test.cpp
 * @brief Unit-tests for the data-upload module
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "data_upload_msg.h"




/*******************************************************************************
*                                  Test Group
*******************************************************************************/
TEST_GROUP( test_data_upload_msg )
{
    char obuff[100];
    /**************************************************************************/
    TEST_SETUP()
    {
        // setup() is run before each test
        memset(obuff, 0, sizeof(obuff));
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




/*******************************************************************************
*                               Test Gateway Message
*******************************************************************************/
TEST( test_data_upload_msg, prepare_gateway_msg1 )
{
    prepare_gateway_msg(obuff, sizeof(obuff));

    STRCMP_EQUAL("gw,fd00::4433:2211,2.0.5,1.000000,-2.000000\r\n", obuff);

    mock().checkExpectations();
}
/******************************************************************************/




/*******************************************************************************
*                                Test Node Message
*******************************************************************************/
TEST( test_data_upload_msg, prepare_node_msg1 )
{
    SensorNode sensor_node;

    SensorNode_init(&sensor_node);

    uip_ip6addr(&sensor_node.ipaddr, 1, 2, 3, 4, 5, 6, 7, 8);

    prepare_node_msg(obuff, sizeof(obuff), &sensor_node);

    STRCMP_EQUAL("nd,1:2:3:4:5:6:7:8\r\n", obuff);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_data_upload_msg, prepare_node_msg2 )
{
    SensorNode sensor_node;

    SensorNode_init(&sensor_node);

    uip_ip6addr(&sensor_node.ipaddr, 0x1234, 0, 0, 0, 5, 6, 7, 8);

    prepare_node_msg(obuff, sizeof(obuff), &sensor_node);

    STRCMP_EQUAL("nd,1234::5:6:7:8\r\n", obuff);

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_data_upload_msg, prepare_node_msg3 )
{
    SensorNode sensor_node;

    SensorNode_init(&sensor_node);

    uip_ip6addr(&sensor_node.ipaddr, 0, 0, 0, 0, 0, 0, 0, 0);

    prepare_node_msg(obuff, sizeof(obuff), &sensor_node);

    STRCMP_EQUAL("nd,::\r\n", obuff);

    mock().checkExpectations();
}
/******************************************************************************/




/*******************************************************************************
*                                Test Data Message
*******************************************************************************/
TEST( test_data_upload_msg, prepare_data_msg1 )
{
    struct SensorData sensor_data;

    memset(&sensor_data, 0, sizeof(sensor_data));

    prepare_data_msg(obuff, sizeof(obuff), &sensor_data);

    STRCMP_EQUAL("da,0.00,0,0,0,0,0,0,0,0,0\r\n", obuff);

    mock().checkExpectations();
}
/******************************************************************************/
