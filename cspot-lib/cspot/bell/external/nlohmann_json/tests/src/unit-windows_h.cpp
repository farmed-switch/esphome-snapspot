

#include "doctest_compatibility.h"
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#ifdef _WIN32
    #include <windows.h>
#endif

#include <nlohmann/json.hpp>
using nlohmann::json;

TEST_CASE("include windows.h")
{
    CHECK(true);
}
