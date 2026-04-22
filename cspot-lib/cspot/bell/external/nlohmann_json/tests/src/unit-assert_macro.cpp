

#include "doctest_compatibility.h"

DOCTEST_GCC_SUPPRESS_WARNING_PUSH
DOCTEST_GCC_SUPPRESS_WARNING("-Wstrict-overflow")
DOCTEST_CLANG_SUPPRESS_WARNING_PUSH
DOCTEST_CLANG_SUPPRESS_WARNING("-Wstrict-overflow")

static int assert_counter;

#define JSON_ASSERT(x) {if (!(x)) ++assert_counter; }

#include <nlohmann/json.hpp>
using nlohmann::json;

#if !defined(JSON_NOEXCEPTION)
TEST_CASE("JSON_ASSERT(x)")
{
    SECTION("basic_json(first, second)")
    {
        assert_counter = 0;
        CHECK(assert_counter == 0);

        const json::iterator it{};
        json j;

        CHECK_THROWS_WITH_AS(json(it, j.end()), "[json.exception.invalid_iterator.201] iterators are not compatible", json::invalid_iterator);

        CHECK(assert_counter == 1);
    }
}
#endif

DOCTEST_GCC_SUPPRESS_WARNING_POP
DOCTEST_CLANG_SUPPRESS_WARNING_POP
