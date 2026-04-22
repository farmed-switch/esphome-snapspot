#ifndef DOCTEST_COMPATIBILITY
#define DOCTEST_COMPATIBILITY

#define DOCTEST_CONFIG_VOID_CAST_EXPRESSIONS
#define DOCTEST_THREAD_LOCAL
#include "doctest.h"

#undef CAPTURE
#define CAPTURE(x) DOCTEST_CAPTURE(x);

#undef SUBCASE
#define SECTION(x) DOCTEST_SUBCASE(x)

#define INFO_WITH_TEMP_IMPL(x, var_name) const auto var_name = x; INFO(var_name)
#define INFO_WITH_TEMP(x) INFO_WITH_TEMP_IMPL(x, DOCTEST_ANONYMOUS(DOCTEST_STD_STRING_))

#define CHECK_THROWS_WITH_STD_STR_IMPL(expr, str, var_name)                    \
    do {                                                                       \
        const std::string var_name = str;                                      \
        CHECK_THROWS_WITH(expr, var_name.c_str());                             \
    } while (false)
#define CHECK_THROWS_WITH_STD_STR(expr, str)                                   \
    CHECK_THROWS_WITH_STD_STR_IMPL(expr, str, DOCTEST_ANONYMOUS(DOCTEST_STD_STRING_))

#include <iosfwd>

using doctest::Approx;

#endif
