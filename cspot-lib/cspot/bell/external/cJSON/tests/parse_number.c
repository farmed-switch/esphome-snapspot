

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"

static cJSON item[1];

static void assert_is_number(cJSON *number_item)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(number_item, "Item is NULL.");

    assert_not_in_list(number_item);
    assert_has_no_child(number_item);
    assert_has_type(number_item, cJSON_Number);
    assert_has_no_reference(number_item);
    assert_has_no_const_string(number_item);
    assert_has_no_valuestring(number_item);
    assert_has_no_string(number_item);
}

static void assert_parse_number(const char *string, int integer, double real)
{
    parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    buffer.content = (const unsigned char*)string;
    buffer.length = strlen(string) + sizeof("");

    TEST_ASSERT_TRUE(parse_number(item, &buffer));
    assert_is_number(item);
    TEST_ASSERT_EQUAL_INT(integer, item->valueint);
    TEST_ASSERT_EQUAL_DOUBLE(real, item->valuedouble);
}

static void parse_number_should_parse_zero(void)
{
    assert_parse_number("0", 0, 0);
    assert_parse_number("0.0", 0, 0.0);
    assert_parse_number("-0", 0, -0.0);
}

static void parse_number_should_parse_negative_integers(void)
{
    assert_parse_number("-1", -1, -1);
    assert_parse_number("-32768", -32768, -32768.0);
    assert_parse_number("-2147483648", (int)-2147483648.0, -2147483648.0);
}

static void parse_number_should_parse_positive_integers(void)
{
    assert_parse_number("1", 1, 1);
    assert_parse_number("32767", 32767, 32767.0);
    assert_parse_number("2147483647", (int)2147483647.0, 2147483647.0);
}

static void parse_number_should_parse_positive_reals(void)
{
    assert_parse_number("0.001", 0, 0.001);
    assert_parse_number("10e-10", 0, 10e-10);
    assert_parse_number("10E-10", 0, 10e-10);
    assert_parse_number("10e10", INT_MAX, 10e10);
    assert_parse_number("123e+127", INT_MAX, 123e127);
    assert_parse_number("123e-128", 0, 123e-128);
}

static void parse_number_should_parse_negative_reals(void)
{
    assert_parse_number("-0.001", 0, -0.001);
    assert_parse_number("-10e-10", 0, -10e-10);
    assert_parse_number("-10E-10", 0, -10e-10);
    assert_parse_number("-10e20", INT_MIN, -10e20);
    assert_parse_number("-123e+127", INT_MIN, -123e127);
    assert_parse_number("-123e-128", 0, -123e-128);
}

int CJSON_CDECL main(void)
{

    memset(item, 0, sizeof(cJSON));
    UNITY_BEGIN();
    RUN_TEST(parse_number_should_parse_zero);
    RUN_TEST(parse_number_should_parse_negative_integers);
    RUN_TEST(parse_number_should_parse_positive_integers);
    RUN_TEST(parse_number_should_parse_positive_reals);
    RUN_TEST(parse_number_should_parse_negative_reals);
    return UNITY_END();
}
