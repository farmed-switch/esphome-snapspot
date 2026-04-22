

#include "doctest_compatibility.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

#include <set>

TEST_CASE("hash<nlohmann::json>")
{

    std::set<std::size_t> hashes;

    hashes.insert(std::hash<json> {}(json(nullptr)));

    hashes.insert(std::hash<json> {}(json(true)));
    hashes.insert(std::hash<json> {}(json(false)));

    hashes.insert(std::hash<json> {}(json("")));
    hashes.insert(std::hash<json> {}(json("foo")));

    hashes.insert(std::hash<json> {}(json(0)));
    hashes.insert(std::hash<json> {}(json(static_cast<unsigned>(0))));

    hashes.insert(std::hash<json> {}(json(-1)));
    hashes.insert(std::hash<json> {}(json(0.0)));
    hashes.insert(std::hash<json> {}(json(42.23)));

    hashes.insert(std::hash<json> {}(json::array()));
    hashes.insert(std::hash<json> {}(json::array({1, 2, 3})));

    hashes.insert(std::hash<json> {}(json::object()));
    hashes.insert(std::hash<json> {}(json::object({{"foo", "bar"}})));

    hashes.insert(std::hash<json> {}(json::binary({})));
    hashes.insert(std::hash<json> {}(json::binary({}, 0)));
    hashes.insert(std::hash<json> {}(json::binary({}, 42)));
    hashes.insert(std::hash<json> {}(json::binary({1, 2, 3})));
    hashes.insert(std::hash<json> {}(json::binary({1, 2, 3}, 0)));
    hashes.insert(std::hash<json> {}(json::binary({1, 2, 3}, 42)));

    hashes.insert(std::hash<json> {}(json(json::value_t::discarded)));

    CHECK(hashes.size() == 21);
}

TEST_CASE("hash<nlohmann::ordered_json>")
{

    std::set<std::size_t> hashes;

    hashes.insert(std::hash<ordered_json> {}(ordered_json(nullptr)));

    hashes.insert(std::hash<ordered_json> {}(ordered_json(true)));
    hashes.insert(std::hash<ordered_json> {}(ordered_json(false)));

    hashes.insert(std::hash<ordered_json> {}(ordered_json("")));
    hashes.insert(std::hash<ordered_json> {}(ordered_json("foo")));

    hashes.insert(std::hash<ordered_json> {}(ordered_json(0)));
    hashes.insert(std::hash<ordered_json> {}(ordered_json(static_cast<unsigned>(0))));

    hashes.insert(std::hash<ordered_json> {}(ordered_json(-1)));
    hashes.insert(std::hash<ordered_json> {}(ordered_json(0.0)));
    hashes.insert(std::hash<ordered_json> {}(ordered_json(42.23)));

    hashes.insert(std::hash<ordered_json> {}(ordered_json::array()));
    hashes.insert(std::hash<ordered_json> {}(ordered_json::array({1, 2, 3})));

    hashes.insert(std::hash<ordered_json> {}(ordered_json::object()));
    hashes.insert(std::hash<ordered_json> {}(ordered_json::object({{"foo", "bar"}})));

    hashes.insert(std::hash<ordered_json> {}(ordered_json::binary({})));
    hashes.insert(std::hash<ordered_json> {}(ordered_json::binary({}, 0)));
    hashes.insert(std::hash<ordered_json> {}(ordered_json::binary({}, 42)));
    hashes.insert(std::hash<ordered_json> {}(ordered_json::binary({1, 2, 3})));
    hashes.insert(std::hash<ordered_json> {}(ordered_json::binary({1, 2, 3}, 0)));
    hashes.insert(std::hash<ordered_json> {}(ordered_json::binary({1, 2, 3}, 42)));

    hashes.insert(std::hash<ordered_json> {}(ordered_json(ordered_json::value_t::discarded)));

    CHECK(hashes.size() == 21);
}
