

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"

static void parse_hex4_should_parse_all_combinations(void)
{
    unsigned int number = 0;
    unsigned char digits_lower[6];
    unsigned char digits_upper[6];

    for (number = 0; number <= 0xFFFF; number++)
    {
        TEST_ASSERT_EQUAL_INT_MESSAGE(4, sprintf((char*)digits_lower, "%.4x", number), "sprintf failed.");
        TEST_ASSERT_EQUAL_INT_MESSAGE(4, sprintf((char*)digits_upper, "%.4X", number), "sprintf failed.");

        TEST_ASSERT_EQUAL_INT_MESSAGE(number, parse_hex4(digits_lower), "Failed to parse lowercase digits.");
        TEST_ASSERT_EQUAL_INT_MESSAGE(number, parse_hex4(digits_upper), "Failed to parse uppercase digits.");
    }
}

static void parse_hex4_should_parse_mixed_case(void)
{
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"beef"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"beeF"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"beEf"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"beEF"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"bEef"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"bEeF"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"bEEf"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"bEEF"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"Beef"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"BeeF"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"BeEf"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"BeEF"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"BEef"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"BEeF"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"BEEf"));
    TEST_ASSERT_EQUAL_INT(0xBEEF, parse_hex4((const unsigned char*)"BEEF"));
}

int CJSON_CDECL main(void)
{
    UNITY_BEGIN();
    RUN_TEST(parse_hex4_should_parse_all_combinations);
    RUN_TEST(parse_hex4_should_parse_mixed_case);
    return UNITY_END();
}
