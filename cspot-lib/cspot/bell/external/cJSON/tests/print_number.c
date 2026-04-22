

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"

static void assert_print_number(const char *expected, double input)
{
    unsigned char printed[1024];
    unsigned char new_buffer[26];
    unsigned int i = 0;
    cJSON item[1];
    printbuffer buffer = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };
    buffer.buffer = printed;
    buffer.length = sizeof(printed);
    buffer.offset = 0;
    buffer.noalloc = true;
    buffer.hooks = global_hooks;
    buffer.buffer = new_buffer;

    memset(item, 0, sizeof(item));
    memset(new_buffer, 0, sizeof(new_buffer));
    cJSON_SetNumberValue(item, input);
    TEST_ASSERT_TRUE_MESSAGE(print_number(item, &buffer), "Failed to print number.");

    for(i = 0;i <sizeof(new_buffer);i++)
    {
        if(i >3 && new_buffer[i] =='0')
        {
            if((new_buffer[i-3] =='e' && new_buffer[i-2] == '-' && new_buffer[i] =='0') ||(new_buffer[i-2] =='e' && new_buffer[i-1] =='+'))
            {
                while(new_buffer[i] !='\0')
                {
                    new_buffer[i] = new_buffer[i+1];
                    i++;
                }
            }
        }
    }
    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, buffer.buffer, "Printed number is not as expected.");
}

static void print_number_should_print_zero(void)
{
    assert_print_number("0", 0);
}

static void print_number_should_print_negative_integers(void)
{
    assert_print_number("-1", -1.0);
    assert_print_number("-32768", -32768.0);
    assert_print_number("-2147483648", -2147483648.0);
}

static void print_number_should_print_positive_integers(void)
{
    assert_print_number("1", 1.0);
    assert_print_number("32767", 32767.0);
    assert_print_number("2147483647", 2147483647.0);
}

static void print_number_should_print_positive_reals(void)
{
    assert_print_number("0.123", 0.123);
    assert_print_number("1e-09", 10e-10);
    assert_print_number("1000000000000", 10e11);
    assert_print_number("1.23e+129", 123e+127);
    assert_print_number("1.23e-126", 123e-128);
    assert_print_number("3.1415926535897931", 3.1415926535897931);
}

static void print_number_should_print_negative_reals(void)
{
    assert_print_number("-0.0123", -0.0123);
    assert_print_number("-1e-09", -10e-10);
    assert_print_number("-1e+21", -10e20);
    assert_print_number("-1.23e+129", -123e+127);
    assert_print_number("-1.23e-126", -123e-128);
}

static void print_number_should_print_non_number(void)
{
    TEST_IGNORE();

}

int CJSON_CDECL main(void)
{

    UNITY_BEGIN();

    RUN_TEST(print_number_should_print_zero);
    RUN_TEST(print_number_should_print_negative_integers);
    RUN_TEST(print_number_should_print_positive_integers);
    RUN_TEST(print_number_should_print_positive_reals);
    RUN_TEST(print_number_should_print_negative_reals);
    RUN_TEST(print_number_should_print_non_number);

    return UNITY_END();
}
