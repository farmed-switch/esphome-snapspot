

#include "doctest_compatibility.h"

#if JSON_TEST_USING_MULTIPLE_HEADERS
    #include <nlohmann/detail/meta/type_traits.hpp>
#else
    #include <nlohmann/json.hpp>
#endif

TEST_CASE("type traits")
{
    SECTION("is_c_string")
    {
        using nlohmann::detail::is_c_string;
        using nlohmann::detail::is_c_string_uncvref;

        SECTION("char *")
        {
            CHECK(is_c_string<char*>::value);
            CHECK(is_c_string<const char*>::value);
            CHECK(is_c_string<char* const>::value);
            CHECK(is_c_string<const char* const>::value);

            CHECK_FALSE(is_c_string<char*&>::value);
            CHECK_FALSE(is_c_string<const char*&>::value);
            CHECK_FALSE(is_c_string<char* const&>::value);
            CHECK_FALSE(is_c_string<const char* const&>::value);

            CHECK(is_c_string_uncvref<char*&>::value);
            CHECK(is_c_string_uncvref<const char*&>::value);
            CHECK(is_c_string_uncvref<char* const&>::value);
            CHECK(is_c_string_uncvref<const char* const&>::value);
        }

        SECTION("char[]")
        {

            CHECK(is_c_string<char[]>::value);
            CHECK(is_c_string<const char[]>::value);

            CHECK_FALSE(is_c_string<char(&)[]>::value);
            CHECK_FALSE(is_c_string<const char(&)[]>::value);

            CHECK(is_c_string_uncvref<char(&)[]>::value);
            CHECK(is_c_string_uncvref<const char(&)[]>::value);

        }
    }
}
