

#include "doctest_compatibility.h"

#include "diag.hpp"

TEST_CASE("ABI compatible diagnostics")
{
    SECTION("basic_json size")
    {

        CHECK(json_sizeof_diag_on() == json_sizeof_diag_on_explicit());
        CHECK(json_sizeof_diag_off() == json_sizeof_diag_off_explicit());
        CHECK(json_sizeof_diag_on() > json_sizeof_diag_off());
    }

    SECTION("basic_json at")
    {

        CHECK_THROWS_WITH(json_at_diag_on(), "[json.exception.out_of_range.403] (/foo) key 'bar' not found");
        CHECK_THROWS_WITH(json_at_diag_off(), "[json.exception.out_of_range.403] key 'bar' not found");
    }
}
