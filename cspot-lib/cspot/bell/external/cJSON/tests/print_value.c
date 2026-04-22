

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"

static void assert_print_value(const char *input)
{
    unsigned char printed[1024];
    cJSON item[1];
    printbuffer buffer = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };
    parse_buffer parsebuffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    buffer.buffer = printed;
    buffer.length = sizeof(printed);
    buffer.offset = 0;
    buffer.noalloc = true;
    buffer.hooks = global_hooks;

    parsebuffer.content = (const unsigned char*)input;
    parsebuffer.length = strlen(input) + sizeof("");
    parsebuffer.hooks = global_hooks;

    memset(item, 0, sizeof(item));

    TEST_ASSERT_TRUE_MESSAGE(parse_value(item, &parsebuffer), "Failed to parse value.");

    TEST_ASSERT_TRUE_MESSAGE(print_value(item, &buffer), "Failed to print value.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE(input, buffer.buffer, "Printed value is not as expected.");

    reset(item);
}

static void print_value_should_print_null(void)
{
    assert_print_value("null");
}

static void print_value_should_print_true(void)
{
    assert_print_value("true");
}

static void print_value_should_print_false(void)
{
    assert_print_value("false");
}

static void print_value_should_print_number(void)
{
    assert_print_value("1.5");
}

static void print_value_should_print_string(void)
{
    assert_print_value("\"\"");
    assert_print_value("\"hello\"");
}

static void print_value_should_print_array(void)
{
    assert_print_value("[]");
}

static void print_value_should_print_object(void)
{
    assert_print_value("{}");
}

int CJSON_CDECL main(void)
{

    UNITY_BEGIN();

    RUN_TEST(print_value_should_print_null);
    RUN_TEST(print_value_should_print_true);
    RUN_TEST(print_value_should_print_false);
    RUN_TEST(print_value_should_print_number);
    RUN_TEST(print_value_should_print_string);
    RUN_TEST(print_value_should_print_array);
    RUN_TEST(print_value_should_print_object);

    return UNITY_END();
}
