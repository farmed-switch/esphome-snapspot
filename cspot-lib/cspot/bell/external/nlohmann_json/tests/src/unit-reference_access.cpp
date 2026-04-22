

#include "doctest_compatibility.h"

#include <nlohmann/json.hpp>
using nlohmann::json;

TEST_CASE("reference access")
{

    const json json_types =
    {
        {"boolean", true},
        {
            "number", {
                {"integer", 42},
                {"floating-point", 17.23}
            }
        },
        {"string", "Hello, world!"},
        {"array", {1, 2, 3, 4, 5}},
        {"null", nullptr}
    };

    SECTION("reference access to object_t")
    {
        using test_type = json::object_t;
        json value = {{"one", 1}, {"two", 2}};

        auto& p1 = value.get_ref<test_type&>();
        CHECK(&p1 == value.get_ptr<test_type*>());
        CHECK(p1 == value.get<test_type>());

        const auto& p2 = value.get_ref<const test_type&>();
        CHECK(&p2 == value.get_ptr<const test_type*>());
        CHECK(p2 == value.get<test_type>());

        CHECK_NOTHROW(value.get_ref<json::object_t&>());
        CHECK_THROWS_WITH_AS(value.get_ref<json::array_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is object", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::string_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is object", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::boolean_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is object", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_integer_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is object", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_unsigned_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is object", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_float_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is object", json::type_error&);
    }

    SECTION("const reference access to const object_t")
    {
        using test_type = json::object_t;
        const json value = {{"one", 1}, {"two", 2}};

        const auto& p2 = value.get_ref<const test_type&>();
        CHECK(&p2 == value.get_ptr<const test_type*>());
        CHECK(p2 == value.get<test_type>());
    }

    SECTION("reference access to array_t")
    {
        using test_type = json::array_t;
        json value = {1, 2, 3, 4};

        auto& p1 = value.get_ref<test_type&>();
        CHECK(&p1 == value.get_ptr<test_type*>());
        CHECK(p1 == value.get<test_type>());

        const auto& p2 = value.get_ref<const test_type&>();
        CHECK(&p2 == value.get_ptr<const test_type*>());
        CHECK(p2 == value.get<test_type>());

        CHECK_THROWS_WITH_AS(value.get_ref<json::object_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is array", json::type_error&);
        CHECK_NOTHROW(value.get_ref<json::array_t&>());
        CHECK_THROWS_WITH_AS(value.get_ref<json::string_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is array", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::boolean_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is array", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_integer_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is array", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_unsigned_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is array", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_float_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is array", json::type_error&);
    }

    SECTION("reference access to string_t")
    {
        using test_type = json::string_t;
        json value = "hello";

        auto& p1 = value.get_ref<test_type&>();
        CHECK(&p1 == value.get_ptr<test_type*>());
        CHECK(p1 == value.get<test_type>());

        const auto& p2 = value.get_ref<const test_type&>();
        CHECK(&p2 == value.get_ptr<const test_type*>());
        CHECK(p2 == value.get<test_type>());

        CHECK_THROWS_WITH_AS(value.get_ref<json::object_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is string", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::array_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is string", json::type_error&);
        CHECK_NOTHROW(value.get_ref<json::string_t&>());
        CHECK_THROWS_WITH_AS(value.get_ref<json::boolean_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is string", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_integer_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is string", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_unsigned_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is string", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_float_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is string", json::type_error&);
    }

    SECTION("reference access to boolean_t")
    {
        using test_type = json::boolean_t;
        json value = false;

        auto& p1 = value.get_ref<test_type&>();
        CHECK(&p1 == value.get_ptr<test_type*>());
        CHECK(p1 == value.get<test_type>());

        const auto& p2 = value.get_ref<const test_type&>();
        CHECK(&p2 == value.get_ptr<const test_type*>());
        CHECK(p2 == value.get<test_type>());

        CHECK_THROWS_WITH_AS(value.get_ref<json::object_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is boolean", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::array_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is boolean", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::string_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is boolean", json::type_error&);
        CHECK_NOTHROW(value.get_ref<json::boolean_t&>());
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_integer_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is boolean", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_unsigned_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is boolean", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_float_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is boolean", json::type_error&);
    }

    SECTION("reference access to number_integer_t")
    {
        using test_type = json::number_integer_t;
        json value = -23;

        auto& p1 = value.get_ref<test_type&>();
        CHECK(&p1 == value.get_ptr<test_type*>());
        CHECK(p1 == value.get<test_type>());

        const auto& p2 = value.get_ref<const test_type&>();
        CHECK(&p2 == value.get_ptr<const test_type*>());
        CHECK(p2 == value.get<test_type>());

        CHECK_THROWS_WITH_AS(value.get_ref<json::object_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::array_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::string_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::boolean_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_NOTHROW(value.get_ref<json::number_integer_t&>());
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_unsigned_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_float_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
    }

    SECTION("reference access to number_unsigned_t")
    {
        using test_type = json::number_unsigned_t;
        json value = 23u;

        auto& p1 = value.get_ref<test_type&>();
        CHECK(&p1 == value.get_ptr<test_type*>());
        CHECK(p1 == value.get<test_type>());

        const auto& p2 = value.get_ref<const test_type&>();
        CHECK(&p2 == value.get_ptr<const test_type*>());
        CHECK(p2 == value.get<test_type>());

        CHECK_THROWS_WITH_AS(value.get_ref<json::object_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::array_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::string_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::boolean_t&>(),
                             "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);

        CHECK_NOTHROW(value.get_ref<json::number_unsigned_t&>());
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_float_t&>(), "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
    }

    SECTION("reference access to number_float_t")
    {
        using test_type = json::number_float_t;
        json value = 42.23;

        auto& p1 = value.get_ref<test_type&>();
        CHECK(&p1 == value.get_ptr<test_type*>());
        CHECK(p1 == value.get<test_type>());

        const auto& p2 = value.get_ref<const test_type&>();
        CHECK(&p2 == value.get_ptr<const test_type*>());
        CHECK(p2 == value.get<test_type>());

        CHECK_THROWS_WITH_AS(value.get_ref<json::object_t&>(), "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::array_t&>(), "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::string_t&>(), "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::boolean_t&>(), "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_integer_t&>(), "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_THROWS_WITH_AS(value.get_ref<json::number_unsigned_t&>(), "[json.exception.type_error.303] incompatible ReferenceType for get_ref, actual type is number", json::type_error&);
        CHECK_NOTHROW(value.get_ref<json::number_float_t&>());
    }
}
