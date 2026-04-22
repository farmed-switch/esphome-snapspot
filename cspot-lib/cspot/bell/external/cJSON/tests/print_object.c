

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"

static void assert_print_object(const char * const expected, const char * const input)
{
    unsigned char printed_unformatted[1024];
    unsigned char printed_formatted[1024];

    cJSON item[1];

    printbuffer formatted_buffer = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };
    printbuffer unformatted_buffer = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };
    parse_buffer parsebuffer = { 0, 0, 0, 0, { 0, 0, 0 } };

    parsebuffer.content = (const unsigned char*)input;
    parsebuffer.length = strlen(input) + sizeof("");
    parsebuffer.hooks = global_hooks;

    formatted_buffer.buffer = printed_formatted;
    formatted_buffer.length = sizeof(printed_formatted);
    formatted_buffer.offset = 0;
    formatted_buffer.noalloc = true;
    formatted_buffer.hooks = global_hooks;

    unformatted_buffer.buffer = printed_unformatted;
    unformatted_buffer.length = sizeof(printed_unformatted);
    unformatted_buffer.offset = 0;
    unformatted_buffer.noalloc = true;
    unformatted_buffer.hooks = global_hooks;

    memset(item, 0, sizeof(item));
    TEST_ASSERT_TRUE_MESSAGE(parse_object(item, &parsebuffer), "Failed to parse object.");

    unformatted_buffer.format = false;
    TEST_ASSERT_TRUE_MESSAGE(print_object(item, &unformatted_buffer), "Failed to print unformatted string.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE(input, printed_unformatted, "Unformatted object is not correct.");

    formatted_buffer.format = true;
    TEST_ASSERT_TRUE_MESSAGE(print_object(item, &formatted_buffer), "Failed to print formatted string.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, printed_formatted, "Formatted ojbect is not correct.");

    reset(item);
}

static void print_object_should_print_empty_objects(void)
{
    assert_print_object("{\n}", "{}");
}

static void print_object_should_print_objects_with_one_element(void)
{

    assert_print_object("{\n\t\"one\":\t1\n}", "{\"one\":1}");
    assert_print_object("{\n\t\"hello\":\t\"world!\"\n}", "{\"hello\":\"world!\"}");
    assert_print_object("{\n\t\"array\":\t[]\n}", "{\"array\":[]}");
    assert_print_object("{\n\t\"null\":\tnull\n}", "{\"null\":null}");
}

static void print_object_should_print_objects_with_multiple_elements(void)
{
    assert_print_object("{\n\t\"one\":\t1,\n\t\"two\":\t2,\n\t\"three\":\t3\n}", "{\"one\":1,\"two\":2,\"three\":3}");
    assert_print_object("{\n\t\"one\":\t1,\n\t\"NULL\":\tnull,\n\t\"TRUE\":\ttrue,\n\t\"FALSE\":\tfalse,\n\t\"array\":\t[],\n\t\"world\":\t\"hello\",\n\t\"object\":\t{\n\t}\n}", "{\"one\":1,\"NULL\":null,\"TRUE\":true,\"FALSE\":false,\"array\":[],\"world\":\"hello\",\"object\":{}}");
}

int CJSON_CDECL main(void)
{

    UNITY_BEGIN();

    RUN_TEST(print_object_should_print_empty_objects);
    RUN_TEST(print_object_should_print_objects_with_one_element);
    RUN_TEST(print_object_should_print_objects_with_multiple_elements);

    return UNITY_END();
}
