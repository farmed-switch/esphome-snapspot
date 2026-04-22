

#include "doctest_compatibility.h"

#include "config.hpp"

#define NLOHMANN_JSON_NAMESPACE nlohmann
#define NLOHMANN_JSON_NAMESPACE_BEGIN namespace nlohmann {
#define NLOHMANN_JSON_NAMESPACE_END }
#include <nlohmann/json_fwd.hpp>

TEST_CASE("custom namespace")
{

#if !DOCTEST_GCC || DOCTEST_GCC >= DOCTEST_COMPILER(4, 9, 0)
    SECTION("namespace matches expectation")
    {
        std::string expected = "nlohmann::basic_json";

        const std::string ns{STRINGIZE(NLOHMANN_JSON_NAMESPACE) "::basic_json"};

        CHECK(namespace_name<nlohmann::json>(ns) == expected);
    }
#endif
}
