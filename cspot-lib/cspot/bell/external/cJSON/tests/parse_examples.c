

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"

static cJSON *parse_file(const char *filename)
{
    cJSON *parsed = NULL;
    char *content = read_file(filename);

    parsed = cJSON_Parse(content);

    if (content != NULL)
    {
        free(content);
    }

    return parsed;
}

static void do_test(const char *test_name)
{
    char *expected = NULL;
    char *actual = NULL;
    cJSON *tree = NULL;

    size_t test_name_length = 0;

    char *test_path = NULL;

    char *expected_path = NULL;

    test_name_length = strlen(test_name);

#define TEST_DIR_PATH "inputs/"
    test_path = (char*)malloc(sizeof(TEST_DIR_PATH) + test_name_length);
    TEST_ASSERT_NOT_NULL_MESSAGE(test_path, "Failed to allocate test_path buffer.");
    expected_path = (char*)malloc(sizeof(TEST_DIR_PATH) + test_name_length + sizeof(".expected"));
    TEST_ASSERT_NOT_NULL_MESSAGE(expected_path, "Failed to allocate expected_path buffer.");

    sprintf(test_path, TEST_DIR_PATH"%s", test_name);
    sprintf(expected_path, TEST_DIR_PATH"%s.expected", test_name);

    expected = read_file(expected_path);
    TEST_ASSERT_NOT_NULL_MESSAGE(expected, "Failed to read expected output.");

    tree = parse_file(test_path);
    TEST_ASSERT_NOT_NULL_MESSAGE(tree, "Failed to read of parse test.");

    actual = cJSON_Print(tree);
    TEST_ASSERT_NOT_NULL_MESSAGE(actual, "Failed to print tree back to JSON.");

    TEST_ASSERT_EQUAL_STRING(expected, actual);

    if (expected != NULL)
    {
        free(expected);
    }
    if (tree != NULL)
    {
        cJSON_Delete(tree);
    }
    if (actual != NULL)
    {
        free(actual);
    }
    if (test_path != NULL)
    {
        free(test_path);
    }
    if (expected_path != NULL)
    {
        free(expected_path);
    }
}

static void file_test1_should_be_parsed_and_printed(void)
{
    do_test("test1");
}

static void file_test2_should_be_parsed_and_printed(void)
{
    do_test("test2");
}

static void file_test3_should_be_parsed_and_printed(void)
{
    do_test("test3");
}

static void file_test4_should_be_parsed_and_printed(void)
{
    do_test("test4");
}

static void file_test5_should_be_parsed_and_printed(void)
{
    do_test("test5");
}

static void file_test6_should_not_be_parsed(void)
{
    char *test6 = NULL;
    cJSON *tree = NULL;

    test6 = read_file("inputs/test6");
    TEST_ASSERT_NOT_NULL_MESSAGE(test6, "Failed to read test6 data.");

    tree = cJSON_Parse(test6);
    TEST_ASSERT_NULL_MESSAGE(tree, "Should fail to parse what is not JSON.");

    TEST_ASSERT_EQUAL_PTR_MESSAGE(test6, cJSON_GetErrorPtr(), "Error pointer is incorrect.");

    if (test6 != NULL)
    {
        free(test6);
    }
    if (tree != NULL)
    {
        cJSON_Delete(tree);
    }
}

static void file_test7_should_be_parsed_and_printed(void)
{
    do_test("test7");
}

static void file_test8_should_be_parsed_and_printed(void)
{
    do_test("test8");
}

static void file_test9_should_be_parsed_and_printed(void)
{
    do_test("test9");
}

static void file_test10_should_be_parsed_and_printed(void)
{
    do_test("test10");
}

static void file_test11_should_be_parsed_and_printed(void)
{
    do_test("test11");
}

static void test12_should_not_be_parsed(void)
{
    const char *test12 = "{ \"name\": ";
    cJSON *tree = NULL;

    tree = cJSON_Parse(test12);
    TEST_ASSERT_NULL_MESSAGE(tree, "Should fail to parse incomplete JSON.");

    TEST_ASSERT_EQUAL_PTR_MESSAGE(test12 + strlen(test12), cJSON_GetErrorPtr(), "Error pointer is incorrect.");

    if (tree != NULL)
    {
        cJSON_Delete(tree);
    }
}

static void test13_should_be_parsed_without_null_termination(void)
{
    cJSON *tree = NULL;
    const char test_13[] = "{" \
                                "\"Image\":{" \
                                    "\"Width\":800," \
                                    "\"Height\":600," \
                                    "\"Title\":\"Viewfrom15thFloor\"," \
                                    "\"Thumbnail\":{" \
                                        "\"Url\":\"http:/*www.example.com/image/481989943\"," \
                                        "\"Height\":125," \
                                        "\"Width\":\"100\"" \
                                    "}," \
                                    "\"IDs\":[116,943,234,38793]" \
                                "}" \
                            "}";

    char test_13_wo_null[sizeof(test_13) - 1];
    memcpy(test_13_wo_null, test_13, sizeof(test_13) - 1);

    tree = cJSON_ParseWithLength(test_13_wo_null, sizeof(test_13) - 1);
    TEST_ASSERT_NOT_NULL_MESSAGE(tree, "Failed to parse valid json.");

    if (tree != NULL)
    {
        cJSON_Delete(tree);
    }
}

static void test14_should_not_be_parsed(void)
{
    cJSON *tree = NULL;
    const char test_14[] = "{" \
                                "\"Image\":{" \
                                    "\"Width\":800," \
                                    "\"Height\":600," \
                                    "\"Title\":\"Viewfrom15thFloor\"," \
                                    "\"Thumbnail\":{" \
                                        "\"Url\":\"http:/*www.example.com/image/481989943\"," \
                                        "\"Height\":125," \
                                        "\"Width\":\"100\"" \
                                    "}," \
                                    "\"IDs\":[116,943,234,38793]" \
                                "}" \
                            "}";

    tree = cJSON_ParseWithLength(test_14, sizeof(test_14) - 2);
    TEST_ASSERT_NULL_MESSAGE(tree, "Should not continue after buffer_length is reached.");

    if (tree != NULL)
    {
        cJSON_Delete(tree);
    }
}

int CJSON_CDECL main(void)
{
    UNITY_BEGIN();
    RUN_TEST(file_test1_should_be_parsed_and_printed);
    RUN_TEST(file_test2_should_be_parsed_and_printed);
    RUN_TEST(file_test3_should_be_parsed_and_printed);
    RUN_TEST(file_test4_should_be_parsed_and_printed);
    RUN_TEST(file_test5_should_be_parsed_and_printed);
    RUN_TEST(file_test6_should_not_be_parsed);
    RUN_TEST(file_test7_should_be_parsed_and_printed);
    RUN_TEST(file_test8_should_be_parsed_and_printed);
    RUN_TEST(file_test9_should_be_parsed_and_printed);
    RUN_TEST(file_test10_should_be_parsed_and_printed);
    RUN_TEST(file_test11_should_be_parsed_and_printed);
    RUN_TEST(test12_should_not_be_parsed);
    RUN_TEST(test13_should_be_parsed_without_null_termination);
    RUN_TEST(test14_should_not_be_parsed);
    return UNITY_END();
}
