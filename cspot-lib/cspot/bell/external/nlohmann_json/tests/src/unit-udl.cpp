

#include "doctest_compatibility.h"

#include <nlohmann/json.hpp>

TEST_CASE("user-defined string literals")
{
    auto j_expected = nlohmann::json::parse(R"({"foo": "bar", "baz": 42})");
    auto ptr_expected = nlohmann::json::json_pointer("/foo/bar");

    SECTION("using namespace nlohmann::literals::json_literals")
    {
        using namespace nlohmann::literals::json_literals;

        CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
        CHECK("/foo/bar"_json_pointer == ptr_expected);
    }

    SECTION("using namespace nlohmann::json_literals")
    {
        using namespace nlohmann::json_literals;

        CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
        CHECK("/foo/bar"_json_pointer == ptr_expected);
    }

    SECTION("using namespace nlohmann::literals")
    {
        using namespace nlohmann::literals;

        CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
        CHECK("/foo/bar"_json_pointer == ptr_expected);
    }

    SECTION("using namespace nlohmann")
    {
        using namespace nlohmann;

        CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
        CHECK("/foo/bar"_json_pointer == ptr_expected);
    }

#ifndef JSON_TEST_NO_GLOBAL_UDLS
    SECTION("global namespace")
    {
        CHECK(R"({"foo": "bar", "baz": 42})"_json == j_expected);
        CHECK("/foo/bar"_json_pointer == ptr_expected);
    }
#endif
}
