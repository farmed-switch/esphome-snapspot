

#include "doctest_compatibility.h"

DOCTEST_GCC_SUPPRESS_WARNING_PUSH
DOCTEST_GCC_SUPPRESS_WARNING("-Wnoexcept")

#include <nlohmann/json.hpp>

using nlohmann::json;

namespace
{
enum test {};

struct pod {};
struct pod_bis {};

void to_json(json&  , pod  ) noexcept;
void to_json(json&  , pod_bis  );
void from_json(const json&  , pod  ) noexcept;
void from_json(const json&  , pod_bis  );
void to_json(json&  , pod  ) noexcept {}
void to_json(json&  , pod_bis  ) {}
void from_json(const json&  , pod  ) noexcept {}
void from_json(const json&  , pod_bis  ) {}

static_assert(noexcept(json{}), "");
static_assert(noexcept(nlohmann::to_json(std::declval<json&>(), 2)), "");
static_assert(noexcept(nlohmann::to_json(std::declval<json&>(), 2.5)), "");
static_assert(noexcept(nlohmann::to_json(std::declval<json&>(), true)), "");
static_assert(noexcept(nlohmann::to_json(std::declval<json&>(), test{})), "");
static_assert(noexcept(nlohmann::to_json(std::declval<json&>(), pod{})), "");
static_assert(!noexcept(nlohmann::to_json(std::declval<json&>(), pod_bis{})), "");
static_assert(noexcept(json(2)), "");
static_assert(noexcept(json(test{})), "");
static_assert(noexcept(json(pod{})), "");
static_assert(noexcept(std::declval<json>().get<pod>()), "");
static_assert(!noexcept(std::declval<json>().get<pod_bis>()), "");
static_assert(noexcept(json(pod{})), "");
}

TEST_CASE("noexcept")
{

    static_cast<void>(static_cast<void(*)(json&, pod)>(&to_json));
    static_cast<void>(static_cast<void(*)(json&, pod_bis)>(&to_json));
    static_cast<void>(static_cast<void(*)(const json&, pod)>(&from_json));
    static_cast<void>(static_cast<void(*)(const json&, pod_bis)>(&from_json));

    SECTION("nothrow-copy-constructible exceptions")
    {

        CHECK(std::is_nothrow_copy_constructible<json::exception>::value == std::is_nothrow_copy_constructible<std::runtime_error>::value);
        CHECK(std::is_nothrow_copy_constructible<json::parse_error>::value == std::is_nothrow_copy_constructible<std::runtime_error>::value);
        CHECK(std::is_nothrow_copy_constructible<json::invalid_iterator>::value == std::is_nothrow_copy_constructible<std::runtime_error>::value);
        CHECK(std::is_nothrow_copy_constructible<json::type_error>::value == std::is_nothrow_copy_constructible<std::runtime_error>::value);
        CHECK(std::is_nothrow_copy_constructible<json::out_of_range>::value == std::is_nothrow_copy_constructible<std::runtime_error>::value);
        CHECK(std::is_nothrow_copy_constructible<json::other_error>::value == std::is_nothrow_copy_constructible<std::runtime_error>::value);
    }
}

DOCTEST_GCC_SUPPRESS_WARNING_POP
