

#include "doctest_compatibility.h"

#define JSON_TESTS_PRIVATE
#include <nlohmann/json.hpp>
using nlohmann::json;

#if JSON_HAS_THREE_WAY_COMPARISON

namespace doctest
{
template<> struct StringMaker<std::partial_ordering>
{
    static String convert(const std::partial_ordering& order)
    {
        if (order == std::partial_ordering::less)
        {
            return "std::partial_ordering::less";
        }
        if (order == std::partial_ordering::equivalent)
        {
            return "std::partial_ordering::equivalent";
        }
        if (order == std::partial_ordering::greater)
        {
            return "std::partial_ordering::greater";
        }
        if (order == std::partial_ordering::unordered)
        {
            return "std::partial_ordering::unordered";
        }
        return "{?}";
    }
};
}
#endif

namespace
{

template <typename A, typename B, typename U = std::less<json::value_t>>
bool f(A a, B b, U u = U())
{
    return u(a, b);
}
}

TEST_CASE("lexicographical comparison operators")
{
    constexpr auto f_ = false;
    constexpr auto _t = true;
    constexpr auto nan = std::numeric_limits<json::number_float_t>::quiet_NaN();
#if JSON_HAS_THREE_WAY_COMPARISON
    constexpr auto lt = std::partial_ordering::less;
    constexpr auto gt = std::partial_ordering::greater;
    constexpr auto eq = std::partial_ordering::equivalent;
    constexpr auto un = std::partial_ordering::unordered;
#endif

#if JSON_HAS_THREE_WAY_COMPARISON
    INFO("using 3-way comparison");
#endif

#if JSON_USE_LEGACY_DISCARDED_VALUE_COMPARISON
    INFO("using legacy comparison");
#endif

    REQUIRE(std::isnan(nan));

    SECTION("types")
    {
        std::vector<json::value_t> j_types =
        {
            json::value_t::null,
            json::value_t::boolean,
            json::value_t::number_integer,
            json::value_t::number_unsigned,
            json::value_t::number_float,
            json::value_t::object,
            json::value_t::array,
            json::value_t::string,
            json::value_t::binary,
            json::value_t::discarded
        };

        std::vector<std::vector<bool>> expected_lt =
        {

            {f_, _t, _t, _t, _t, _t, _t, _t, _t, f_},
            {f_, f_, _t, _t, _t, _t, _t, _t, _t, f_},
            {f_, f_, f_, f_, f_, _t, _t, _t, _t, f_},
            {f_, f_, f_, f_, f_, _t, _t, _t, _t, f_},
            {f_, f_, f_, f_, f_, _t, _t, _t, _t, f_},
            {f_, f_, f_, f_, f_, f_, _t, _t, _t, f_},
            {f_, f_, f_, f_, f_, f_, f_, _t, _t, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, _t, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
        };

        SECTION("comparison: less")
        {
            REQUIRE(expected_lt.size() == j_types.size());
            for (size_t i = 0; i < j_types.size(); ++i)
            {
                REQUIRE(expected_lt[i].size() == j_types.size());
                for (size_t j = 0; j < j_types.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)

#if JSON_HAS_THREE_WAY_COMPARISON

                    CHECK((j_types[i] < j_types[j]) == expected_lt[i][j]);
#else
                    CHECK(operator<(j_types[i], j_types[j]) == expected_lt[i][j]);
#endif
                    CHECK(f(j_types[i], j_types[j]) == expected_lt[i][j]);
                }
            }
        }
#if JSON_HAS_THREE_WAY_COMPARISON

        SECTION("comparison: 3-way")
        {
            std::vector<std::vector<std::partial_ordering>> expected =
            {

                {eq, lt, lt, lt, lt, lt, lt, lt, lt, un},
                {gt, eq, lt, lt, lt, lt, lt, lt, lt, un},
                {gt, gt, eq, eq, eq, lt, lt, lt, lt, un},
                {gt, gt, eq, eq, eq, lt, lt, lt, lt, un},
                {gt, gt, eq, eq, eq, lt, lt, lt, lt, un},
                {gt, gt, gt, gt, gt, eq, lt, lt, lt, un},
                {gt, gt, gt, gt, gt, gt, eq, lt, lt, un},
                {gt, gt, gt, gt, gt, gt, gt, eq, lt, un},
                {gt, gt, gt, gt, gt, gt, gt, gt, eq, un},
                {un, un, un, un, un, un, un, un, un, un},
            };

            REQUIRE(expected.size() == expected_lt.size());
            for (size_t i = 0; i < expected.size(); ++i)
            {
                REQUIRE(expected[i].size() == expected_lt[i].size());
                for (size_t j = 0; j < expected[i].size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    CHECK(std::is_lt(expected[i][j]) == expected_lt[i][j]);
                }
            }

            REQUIRE(expected.size() == j_types.size());
            for (size_t i = 0; i < j_types.size(); ++i)
            {
                REQUIRE(expected[i].size() == j_types.size());
                for (size_t j = 0; j < j_types.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    CHECK((j_types[i] <=> j_types[j]) == expected[i][j]);
                }
            }
        }
#endif
    }

    SECTION("values")
    {
        json j_values =
        {
            nullptr, nullptr,
            -17, 42,
            8u, 13u,
            3.14159, 23.42,
            nan, nan,
            "foo", "bar",
            true, false,
            {1, 2, 3}, {"one", "two", "three"},
            {{"first", 1}, {"second", 2}}, {{"a", "A"}, {"b", {"B"}}},
            json::binary({1, 2, 3}), json::binary({1, 2, 4}),
            json(json::value_t::discarded), json(json::value_t::discarded)
        };

        std::vector<std::vector<bool>> expected_eq =
        {

            {_t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {_t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
        };

        std::vector<std::vector<bool>> expected_lt =
        {

            {f_, f_, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, _t, _t, _t, _t, _t, f_, f_, _t, _t, f_, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, _t, f_, _t, f_, _t, f_, f_, _t, _t, f_, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, _t, f_, f_, f_, _t, f_, f_, _t, _t, f_, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, _t, _t, _t, f_, _t, f_, f_, _t, _t, f_, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_},
            {f_, f_, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, f_, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, f_, _t, _t, _t, _t, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, _t, f_, f_, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, _t, _t, f_, f_, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, _t, _t, _t, f_, _t, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
        };

        SECTION("compares unordered")
        {
            std::vector<std::vector<bool>> expected =
            {

                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, _t, _t, _t, _t, _t, _t, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, _t, _t, _t, _t, _t, _t, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, _t, _t},
                {_t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t},
                {_t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t, _t},
            };

            REQUIRE(expected.size() == j_values.size());
            for (size_t i = 0; i < j_values.size(); ++i)
            {
                REQUIRE(expected[i].size() == j_values.size());
                for (size_t j = 0; j < j_values.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    CHECK(json::compares_unordered(j_values[i], j_values[j]) == expected[i][j]);
                }
            }
        }

#if JSON_USE_LEGACY_DISCARDED_VALUE_COMPARISON
        SECTION("compares unordered (inverse)")
        {
            std::vector<std::vector<bool>> expected =
            {

                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, _t, _t, _t, _t, _t, _t, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, _t, _t, _t, _t, _t, _t, _t, _t, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
                {f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_, f_},
            };

            REQUIRE(expected.size() == j_values.size());
            for (size_t i = 0; i < j_values.size(); ++i)
            {
                REQUIRE(expected[i].size() == j_values.size());
                for (size_t j = 0; j < j_values.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    CAPTURE(j_values[i])
                    CAPTURE(j_values[j])
                    CHECK(json::compares_unordered(j_values[i], j_values[j], true) == expected[i][j]);
                }
            }
        }
#endif

        SECTION("comparison: equal")
        {

            REQUIRE(expected_eq.size() == j_values.size());
            for (size_t i = 0; i < j_values.size(); ++i)
            {
                REQUIRE(expected_eq[i].size() == j_values.size());
                for (size_t j = 0; j < j_values.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    CHECK((j_values[i] == j_values[j]) == expected_eq[i][j]);
                }
            }

            json j_null;
            CHECK(j_null == nullptr);
            CHECK(nullptr == j_null);
        }

        SECTION("comparison: not equal")
        {

            for (size_t i = 0; i < j_values.size(); ++i)
            {
                for (size_t j = 0; j < j_values.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)

                    if (json::compares_unordered(j_values[i], j_values[j], true))
                    {

                        CHECK_FALSE(j_values[i] != j_values[j]);
                    }
                    else
                    {

                        CHECK((j_values[i] != j_values[j]) == !(j_values[i] == j_values[j]));
                    }
                }
            }

            const json j_null;
            CHECK((j_null != nullptr) == false);
            CHECK((nullptr != j_null) == false);
            CHECK((j_null != nullptr) == !(j_null == nullptr));
            CHECK((nullptr != j_null) == !(nullptr == j_null));
        }

        SECTION("comparison: less")
        {

            REQUIRE(expected_lt.size() == j_values.size());
            for (size_t i = 0; i < j_values.size(); ++i)
            {
                REQUIRE(expected_lt[i].size() == j_values.size());
                for (size_t j = 0; j < j_values.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    CHECK((j_values[i] < j_values[j]) == expected_lt[i][j]);
                }
            }
        }

        SECTION("comparison: less than or equal equal")
        {

            for (size_t i = 0; i < j_values.size(); ++i)
            {
                for (size_t j = 0; j < j_values.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    if (json::compares_unordered(j_values[i], j_values[j], true))
                    {

                        CHECK_FALSE(j_values[i] <= j_values[j]);
                    }
                    else
                    {

                        CHECK((j_values[i] <= j_values[j]) == !(j_values[j] < j_values[i]));
                    }
                }
            }
        }

        SECTION("comparison: greater than")
        {

            for (size_t i = 0; i < j_values.size(); ++i)
            {
                for (size_t j = 0; j < j_values.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    if (json::compares_unordered(j_values[i], j_values[j]))
                    {

                        CHECK_FALSE(j_values[i] > j_values[j]);
                    }
                    else
                    {

                        CHECK((j_values[i] > j_values[j]) == !(j_values[i] <= j_values[j]));
                        CHECK((j_values[i] > j_values[j]) == !!(j_values[j] < j_values[i]));
                    }
                }
            }
        }

        SECTION("comparison: greater than or equal")
        {

            for (size_t i = 0; i < j_values.size(); ++i)
            {
                for (size_t j = 0; j < j_values.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    if (json::compares_unordered(j_values[i], j_values[j], true))
                    {

                        CHECK_FALSE(j_values[i] >= j_values[j]);
                    }
                    else
                    {

                        CHECK((j_values[i] >= j_values[j]) == !(j_values[i] < j_values[j]));
                    }
                }
            }
        }

#if JSON_HAS_THREE_WAY_COMPARISON

        SECTION("comparison: 3-way")
        {
            std::vector<std::vector<std::partial_ordering>> expected =
            {

                {eq, eq, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, un, un},
                {eq, eq, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, eq, lt, lt, lt, lt, lt, un, un, lt, lt, gt, gt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, gt, eq, gt, gt, gt, gt, un, un, lt, lt, gt, gt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, gt, lt, eq, lt, gt, lt, un, un, lt, lt, gt, gt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, gt, lt, gt, eq, gt, lt, un, un, lt, lt, gt, gt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, gt, lt, lt, lt, eq, lt, un, un, lt, lt, gt, gt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, gt, lt, gt, gt, gt, eq, un, un, lt, lt, gt, gt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, un, un, un, un, un, un, un, un, lt, lt, gt, gt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, un, un, un, un, un, un, un, un, lt, lt, gt, gt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, eq, gt, gt, gt, gt, gt, gt, gt, lt, lt, un, un},
                {gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, lt, eq, gt, gt, gt, gt, gt, gt, lt, lt, un, un},
                {gt, gt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, eq, gt, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, eq, lt, lt, lt, lt, lt, lt, un, un},
                {gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, lt, lt, gt, gt, eq, lt, gt, gt, lt, lt, un, un},
                {gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, lt, lt, gt, gt, gt, eq, gt, gt, lt, lt, un, un},
                {gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, lt, lt, gt, gt, lt, lt, eq, gt, lt, lt, un, un},
                {gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, lt, lt, gt, gt, lt, lt, lt, eq, lt, lt, un, un},
                {gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, eq, lt, un, un},
                {gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, gt, eq, un, un},
                {un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un},
                {un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un, un},
            };

            REQUIRE(expected.size() == expected_eq.size());
            REQUIRE(expected.size() == expected_lt.size());
            for (size_t i = 0; i < expected.size(); ++i)
            {
                REQUIRE(expected[i].size() == expected_eq[i].size());
                REQUIRE(expected[i].size() == expected_lt[i].size());
                for (size_t j = 0; j < expected[i].size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    CHECK(std::is_eq(expected[i][j]) == expected_eq[i][j]);
                    CHECK(std::is_lt(expected[i][j]) == expected_lt[i][j]);
                    if (std::is_gt(expected[i][j]))
                    {
                        CHECK((!expected_eq[i][j] && !expected_lt[i][j]));
                    }
                }
            }

            REQUIRE(expected.size() == j_values.size());
            for (size_t i = 0; i < j_values.size(); ++i)
            {
                REQUIRE(expected[i].size() == j_values.size());
                for (size_t j = 0; j < j_values.size(); ++j)
                {
                    CAPTURE(i)
                    CAPTURE(j)
                    CHECK((j_values[i] <=> j_values[j]) == expected[i][j]);
                }
            }
        }
#endif
    }

#if JSON_USE_LEGACY_DISCARDED_VALUE_COMPARISON
    SECTION("parser callback regression")
    {
        SECTION("filter specific element")
        {
            const auto* s_object = R"(
                {
                    "foo": 2,
                    "bar": {
                        "baz": 1
                    }
                }
            )";
            const auto* s_array = R"(
                [1,2,[3,4,5],4,5]
            )";

            json j_object = json::parse(s_object, [](int  , json::parse_event_t  , const json & j) noexcept
            {

                return j != json(2);
            });

            CHECK (j_object == json({{"bar", {{"baz", 1}}}}));

            json j_array = json::parse(s_array, [](int  , json::parse_event_t  , const json & j) noexcept
            {
                return j != json(2);
            });

            CHECK (j_array == json({1, {3, 4, 5}, 4, 5}));
        }
    }
#endif
}
