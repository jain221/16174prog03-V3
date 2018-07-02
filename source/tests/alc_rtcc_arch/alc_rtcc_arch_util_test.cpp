/**
 * @file  alc_rtcc_arch_util_test.cpp
 * @brief Test the functions in the 'alc_rtcc_arch_util.c' file.
 *
 * @note
 * Copyright (C) 2016, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "alc_rtcc_arch.h"


/*******************************************************************************
*                                  Test Group
*******************************************************************************/
TEST_GROUP( test_alc_rtcc_arch_util )
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




/*******************************************************************************
*                                    Tests
*******************************************************************************/
TEST( test_alc_rtcc_arch_util, zero )
{
    LONGS_EQUAL(0, AlcRtcc_calc_trim(0U) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_alc_rtcc_arch_util, positive )
{
    LONGS_EQUAL(1, AlcRtcc_calc_trim(1U) );
    LONGS_EQUAL(2, AlcRtcc_calc_trim(2U) );
    LONGS_EQUAL(3, AlcRtcc_calc_trim(3U) );

    LONGS_EQUAL(32765, AlcRtcc_calc_trim(0x7FFDU) );
    LONGS_EQUAL(32766, AlcRtcc_calc_trim(0x7FFEU) );
    LONGS_EQUAL(32767, AlcRtcc_calc_trim(0x7FFFU) );

    mock().checkExpectations();
}
/******************************************************************************/
TEST( test_alc_rtcc_arch_util, negative )
{
    LONGS_EQUAL(-1, AlcRtcc_calc_trim(0xFFFFU) );
    LONGS_EQUAL(-2, AlcRtcc_calc_trim(0xFFFEU) );
    LONGS_EQUAL(-3, AlcRtcc_calc_trim(0xFFFDU) );

    LONGS_EQUAL(-32766, AlcRtcc_calc_trim(0x8002U) );
    LONGS_EQUAL(-32767, AlcRtcc_calc_trim(0x8001U) );
    LONGS_EQUAL(-32768, AlcRtcc_calc_trim(0x8000U) );

    mock().checkExpectations();
}
/******************************************************************************/
