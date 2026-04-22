

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"

static cJSON item[1];

static void assert_is_value(cJSON *value_item, int type)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(value_item, "Item is NULL.");

    assert_not_in_list(value_item);
    assert_has_type(value_item, type);
    assert_has_no_reference(value_item);
    assert_has_no_const_string(value_item);
    assert_has_no_string(value_item);
}

static void assert_parse_value(const char *string, int type)
{
    parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    buffer.content = (const unsigned char*) string;
    buffer.length = strlen(string) + sizeof("");
    buffer.hooks = global_hooks;

    TEST_ASSERT_TRUE(parse_value(item, &buffer));
    assert_is_value(item, type);
}

static void parse_value_should_parse_null(void)
{
    assert_parse_value("null", cJSON_NULL);
    reset(item);
}

static void parse_value_should_parse_true(void)
{
    assert_parse_value("true", cJSON_True);
    reset(item);
}

static void parse_value_should_parse_false(void)
{
    assert_parse_value("false", cJSON_False);
    reset(item);
}

static void parse_value_should_parse_number(void)
{
    assert_parse_value("1.5", cJSON_Number);
    reset(item);
}

static void parse_value_should_parse_string(void)
{
    assert_parse_value("\"\"", cJSON_String);
    reset(item);
    assert_parse_value("\"hello\"", cJSON_String);
    reset(item);
}

static void parse_value_should_parse_array(void)
{
    assert_parse_value("[]", cJSON_Array);
    reset(item);
}

static void parse_value_should_parse_object(void)
{
    assert_parse_value("{}", cJSON_Object);
    reset(item);
}

int CJSON_CDECL main(void)
{

    memset(item, 0, sizeof(cJSON));
    UNITY_BEGIN();
    RUN_TEST(parse_value_should_parse_null);
    RUN_TEST(parse_value_should_parse_true);
    RUN_TEST(parse_value_should_parse_false);
    RUN_TEST(parse_value_should_parse_number);
    RUN_TEST(parse_value_should_parse_string);
    RUN_TEST(parse_value_should_parse_array);
    RUN_TEST(parse_value_should_parse_object);
    return UNITY_END();
}
