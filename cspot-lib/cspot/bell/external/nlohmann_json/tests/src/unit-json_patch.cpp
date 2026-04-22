

#include "doctest_compatibility.h"

#include <nlohmann/json.hpp>
using nlohmann::json;
#ifdef JSON_TEST_NO_GLOBAL_UDLS
    using namespace nlohmann::literals;
#endif

#include <fstream>
#include "make_test_data_available.hpp"

TEST_CASE("JSON patch")
{
    SECTION("examples from RFC 6902")
    {
        SECTION("4. Operations")
        {

            json op1 = R"({ "op": "add", "path": "/a/b/c", "value": "foo" })"_json;
            json op2 = R"({ "path": "/a/b/c", "op": "add", "value": "foo" })"_json;
            json op3 = R"({ "value": "foo", "path": "/a/b/c", "op": "add" })"_json;

            CHECK(op1 == op2);
            CHECK(op1 == op3);
        }

        SECTION("4.1 add")
        {
            json const patch1 = R"([{ "op": "add", "path": "/a/b", "value": [ "foo", "bar" ] }])"_json;

            json const doc1 = R"({ "a": { "foo": 1 } })"_json;

            CHECK_NOTHROW(doc1.patch(patch1));
            auto doc1_ans = R"(
                {
                    "a": {
                        "foo": 1,
                        "b": [ "foo", "bar" ]
                    }
                }
            )"_json;
            CHECK(doc1.patch(patch1) == doc1_ans);

            json const doc2 = R"({ "q": { "bar": 2 } })"_json;

            CHECK_THROWS_WITH_AS(doc2.patch(patch1), "[json.exception.out_of_range.403] key 'a' not found", json::out_of_range&);

            json const doc3 = R"({ "a": {} })"_json;
            json const patch2 = R"([{ "op": "add", "path": "/a/b/c", "value": 1 }])"_json;

#if JSON_DIAGNOSTICS
            CHECK_THROWS_WITH_AS(doc3.patch(patch2), "[json.exception.out_of_range.403] (/a) key 'b' not found", json::out_of_range&);
#else
            CHECK_THROWS_WITH_AS(doc3.patch(patch2), "[json.exception.out_of_range.403] key 'b' not found", json::out_of_range&);
#endif
        }

        SECTION("4.2 remove")
        {

            json const doc = {1, 2, 3, 4};
            json const patch = {{{"op", "remove"}, {"path", "/1"}}};
            CHECK(doc.patch(patch) == json({1, 3, 4}));
        }

        SECTION("A.1. Adding an Object Member")
        {

            json const doc = R"(
                    { "foo": "bar"}
                )"_json;

            json const patch = R"(
                    [
                        { "op": "add", "path": "/baz", "value": "qux" }
                    ]
                )"_json;

            json expected = R"(
                    {
                        "baz": "qux",
                        "foo": "bar"
                    }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.2. Adding an Array Element")
        {

            json const doc = R"(
                    { "foo": [ "bar", "baz" ] }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "add", "path": "/foo/1", "value": "qux" }
                    ]
                )"_json;

            json expected = R"(
                    { "foo": [ "bar", "qux", "baz" ] }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.3. Removing an Object Member")
        {

            json const doc = R"(
                    {
                        "baz": "qux",
                        "foo": "bar"
                    }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "remove", "path": "/baz" }
                    ]
                )"_json;

            json expected = R"(
                    { "foo": "bar" }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.4. Removing an Array Element")
        {

            json const doc = R"(
                    { "foo": [ "bar", "qux", "baz" ] }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "remove", "path": "/foo/1" }
                    ]
                )"_json;

            json expected = R"(
                    { "foo": [ "bar", "baz" ] }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.5. Replacing a Value")
        {

            json const doc = R"(
                    {
                        "baz": "qux",
                        "foo": "bar"
                    }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "replace", "path": "/baz", "value": "boo" }
                    ]
                )"_json;

            json expected = R"(
                    {
                        "baz": "boo",
                        "foo": "bar"
                    }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.6. Moving a Value")
        {

            json const doc = R"(
                    {
                        "foo": {
                           "bar": "baz",
                            "waldo": "fred"
                        },
                        "qux": {
                            "corge": "grault"
                        }
                    }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "move", "from": "/foo/waldo", "path": "/qux/thud" }
                    ]
                )"_json;

            json expected = R"(
                    {
                        "foo": {
                           "bar": "baz"
                        },
                        "qux": {
                            "corge": "grault",
                            "thud": "fred"
                        }
                    }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.7. Moving a Value")
        {

            json const doc = R"(
                    { "foo": [ "all", "grass", "cows", "eat" ] }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "move", "from": "/foo/1", "path": "/foo/3" }
                    ]
                )"_json;

            json expected = R"(
                    { "foo": [ "all", "cows", "eat", "grass" ] }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.8. Testing a Value: Success")
        {

            json doc = R"(
                    {
                         "baz": "qux",
                         "foo": [ "a", 2, "c" ]
                    }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "test", "path": "/baz", "value": "qux" },
                        { "op": "test", "path": "/foo/1", "value": 2 }
                    ]
                )"_json;

            CHECK_NOTHROW(doc.patch(patch));

            CHECK(doc.patch(patch) == doc);
        }

        SECTION("A.9. Testing a Value: Error")
        {

            json const doc = R"(
                    { "baz": "qux" }
                )"_json;

            json patch = R"(
                    [
                        { "op": "test", "path": "/baz", "value": "bar" }
                    ]
                )"_json;

            CHECK_THROWS_AS(doc.patch(patch), json::other_error&);
#if JSON_DIAGNOSTICS
            CHECK_THROWS_WITH_STD_STR(doc.patch(patch), "[json.exception.other_error.501] (/0) unsuccessful: " + patch[0].dump());
#else
            CHECK_THROWS_WITH_STD_STR(doc.patch(patch), "[json.exception.other_error.501] unsuccessful: " + patch[0].dump());
#endif
        }

        SECTION("A.10. Adding a Nested Member Object")
        {

            json const doc = R"(
                    { "foo": "bar" }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "add", "path": "/child", "value": { "grandchild": { } } }
                    ]
                )"_json;

            json expected = R"(
                {
                    "foo": "bar",
                    "child": {
                        "grandchild": {
                        }
                    }
                }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.11. Ignoring Unrecognized Elements")
        {

            json const doc = R"(
                    { "foo": "bar" }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "add", "path": "/baz", "value": "qux", "xyz": 123 }
                    ]
                )"_json;

            json expected = R"(
                    {
                        "foo": "bar",
                        "baz": "qux"
                    }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.12. Adding to a Nonexistent Target")
        {

            json const doc = R"(
                    { "foo": "bar" }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "add", "path": "/baz/bat", "value": "qux" }
                    ]
                )"_json;

            CHECK_THROWS_WITH_AS(doc.patch(patch), "[json.exception.out_of_range.403] key 'baz' not found", json::out_of_range&);
        }

        SECTION("A.14. Escape Ordering")
        {

            json const doc = R"(
                    {
                        "/": 9,
                        "~1": 10
                    }
                )"_json;

            json const patch = R"(
                    [
                        {"op": "test", "path": "/~01", "value": 10}
                    ]
                )"_json;

            json expected = R"(
                    {
                        "/": 9,
                        "~1": 10
                    }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("A.15. Comparing Strings and Numbers")
        {

            json const doc = R"(
                    {
                        "/": 9,
                        "~1": 10
                    }
                )"_json;

            json patch = R"(
                    [
                        {"op": "test", "path": "/~01", "value": "10"}
                    ]
                )"_json;

            CHECK_THROWS_AS(doc.patch(patch), json::other_error&);
#if JSON_DIAGNOSTICS
            CHECK_THROWS_WITH_STD_STR(doc.patch(patch), "[json.exception.other_error.501] (/0) unsuccessful: " + patch[0].dump());
#else
            CHECK_THROWS_WITH_STD_STR(doc.patch(patch), "[json.exception.other_error.501] unsuccessful: " + patch[0].dump());
#endif
        }

        SECTION("A.16. Adding an Array Value")
        {

            json const doc = R"(
                    { "foo": ["bar"] }
                )"_json;

            json const patch = R"(
                    [
                        { "op": "add", "path": "/foo/-", "value": ["abc", "def"] }
                    ]
                )"_json;

            json expected = R"(
                    { "foo": ["bar", ["abc", "def"]] }
                )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }
    }

    SECTION("own examples")
    {
        SECTION("add")
        {
            SECTION("add to the root element")
            {

                json const doc = 17;

                json const patch = R"(
                        [
                            { "op": "add", "path": "", "value": [1,2,3] }
                        ]
                    )"_json;

                json expected = {1, 2, 3};

                CHECK(doc.patch(patch) == expected);

                CHECK(doc.patch(json::diff(doc, expected)) == expected);
            }

            SECTION("add to end of the array")
            {

                json const doc = {0, 1, 2};

                json const patch = R"(
                    [
                        { "op": "add", "path": "/3", "value": 3 }
                    ]
                )"_json;

                json expected = {0, 1, 2, 3};

                CHECK(doc.patch(patch) == expected);

                CHECK(doc.patch(json::diff(doc, expected)) == expected);
            }
        }

        SECTION("copy")
        {

            json const doc = R"(
                {
                    "foo": {
                        "bar": "baz",
                        "waldo": "fred"
                    },
                    "qux": {
                       "corge": "grault"
                    }
                }
            )"_json;

            json const patch = R"(
                [
                    { "op": "copy", "from": "/foo/waldo", "path": "/qux/thud" }
                ]
            )"_json;

            json expected = R"(
                {
                    "foo": {
                        "bar": "baz",
                        "waldo": "fred"
                    },
                    "qux": {
                       "corge": "grault",
                       "thud": "fred"
                    }
                }
            )"_json;

            CHECK(doc.patch(patch) == expected);

            CHECK(doc.patch(json::diff(doc, expected)) == expected);
        }

        SECTION("replace")
        {
            json const j = "string";
            json const patch = {{{"op", "replace"}, {"path", ""}, {"value", 1}}};
            CHECK(j.patch(patch) == json(1));
        }

        SECTION("documentation GIF")
        {
            {

                json const p1 = R"(
                     [{"op": "add", "path": "/GB", "value": "London"}]
                    )"_json;

                json const source = R"(
                      {"D": "Berlin", "F": "Paris"}
                    )"_json;

                json target = source.patch(p1);

                CHECK(target == R"({ "D": "Berlin", "F": "Paris", "GB": "London" })"_json);

                json p2 = json::diff(target, source);

                CHECK(p2 == R"([{"op":"remove","path":"/GB"}])"_json);
            }
            {

                json j = {"good", "bad", "ugly"};

                auto ptr = json::json_pointer("/2");

                j[ptr] = {{"it", "cattivo"}};
                CHECK(j == R"(["good","bad",{"it":"cattivo"}])"_json);

                j["/2/en"_json_pointer] = "ugly";
                CHECK(j == R"(["good","bad",{"en":"ugly","it":"cattivo"}])"_json);

                json flat = j.flatten();
                CHECK(flat == R"({"/0":"good","/1":"bad","/2/en":"ugly","/2/it":"cattivo"})"_json);
            }
        }
    }

    SECTION("errors")
    {
        SECTION("unknown operation")
        {
            SECTION("not an array")
            {
                json const j;
                json const patch = {{"op", "add"}, {"path", ""}, {"value", 1}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.104] parse error: JSON patch must be an array of objects", json::parse_error&);
            }

            SECTION("not an array of objects")
            {
                json const j;
                json const patch = {"op", "add", "path", "", "value", 1};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.104] parse error: (/0) JSON patch must be an array of objects", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.104] parse error: JSON patch must be an array of objects", json::parse_error&);
#endif
            }

            SECTION("missing 'op'")
            {
                json const j;
                json const patch = {{{"foo", "bar"}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation must have member 'op'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation must have member 'op'", json::parse_error&);
#endif
            }

            SECTION("non-string 'op'")
            {
                json const j;
                json const patch = {{{"op", 1}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation must have string member 'op'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation must have string member 'op'", json::parse_error&);
#endif
            }

            SECTION("invalid operation")
            {
                json const j;
                json const patch = {{{"op", "foo"}, {"path", ""}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation value 'foo' is invalid", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation value 'foo' is invalid", json::parse_error&);
#endif
            }
        }

        SECTION("add")
        {
            SECTION("missing 'path'")
            {
                json const j;
                json const patch = {{{"op", "add"}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'add' must have member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'add' must have member 'path'", json::parse_error&);
#endif
            }

            SECTION("non-string 'path'")
            {
                json const j;
                json const patch = {{{"op", "add"}, {"path", 1}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'add' must have string member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'add' must have string member 'path'", json::parse_error&);
#endif
            }

            SECTION("missing 'value'")
            {
                json const j;
                json const patch = {{{"op", "add"}, {"path", ""}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'add' must have member 'value'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'add' must have member 'value'", json::parse_error&);
#endif
            }

            SECTION("invalid array index")
            {
                json const j = {1, 2};
                json const patch = {{{"op", "add"}, {"path", "/4"}, {"value", 4}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.401] array index 4 is out of range", json::out_of_range&);
            }
        }

        SECTION("remove")
        {
            SECTION("missing 'path'")
            {
                json const j;
                json const patch = {{{"op", "remove"}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'remove' must have member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'remove' must have member 'path'", json::parse_error&);
#endif
            }

            SECTION("non-string 'path'")
            {
                json const j;
                json const patch = {{{"op", "remove"}, {"path", 1}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'remove' must have string member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'remove' must have string member 'path'", json::parse_error&);
#endif
            }

            SECTION("nonexisting target location (array)")
            {
                json const j = {1, 2, 3};
                json const patch = {{{"op", "remove"}, {"path", "/17"}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.401] array index 17 is out of range", json::out_of_range&);
            }

            SECTION("nonexisting target location (object)")
            {
                json const j = {{"foo", 1}, {"bar", 2}};
                json const patch = {{{"op", "remove"}, {"path", "/baz"}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.403] key 'baz' not found", json::out_of_range&);
            }

            SECTION("root element as target location")
            {
                json const j = "string";
                json const patch = {{{"op", "remove"}, {"path", ""}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.405] JSON pointer has no parent", json::out_of_range&);
            }
        }

        SECTION("replace")
        {
            SECTION("missing 'path'")
            {
                json const j;
                json const patch = {{{"op", "replace"}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'replace' must have member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'replace' must have member 'path'", json::parse_error&);
#endif
            }

            SECTION("non-string 'path'")
            {
                json const j;
                json const patch = {{{"op", "replace"}, {"path", 1}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'replace' must have string member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'replace' must have string member 'path'", json::parse_error&);
#endif
            }

            SECTION("missing 'value'")
            {
                json const j;
                json const patch = {{{"op", "replace"}, {"path", ""}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'replace' must have member 'value'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'replace' must have member 'value'", json::parse_error&);
#endif
            }

            SECTION("nonexisting target location (array)")
            {
                json const j = {1, 2, 3};
                json const patch = {{{"op", "replace"}, {"path", "/17"}, {"value", 19}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.401] array index 17 is out of range", json::out_of_range&);
            }

            SECTION("nonexisting target location (object)")
            {
                json const j = {{"foo", 1}, {"bar", 2}};
                json const patch = {{{"op", "replace"}, {"path", "/baz"}, {"value", 3}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.403] key 'baz' not found", json::out_of_range&);
            }
        }

        SECTION("move")
        {
            SECTION("missing 'path'")
            {
                json const j;
                json const patch = {{{"op", "move"}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'move' must have member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'move' must have member 'path'", json::parse_error&);
#endif
            }

            SECTION("non-string 'path'")
            {
                json const j;
                json const patch = {{{"op", "move"}, {"path", 1}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'move' must have string member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'move' must have string member 'path'", json::parse_error&);
#endif
            }

            SECTION("missing 'from'")
            {
                json const j;
                json const patch = {{{"op", "move"}, {"path", ""}}};
                CHECK_THROWS_AS(j.patch(patch), json::parse_error&);
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'move' must have member 'from'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'move' must have member 'from'", json::parse_error&);
#endif
            }

            SECTION("non-string 'from'")
            {
                json const j;
                json const patch = {{{"op", "move"}, {"path", ""}, {"from", 1}}};
                CHECK_THROWS_AS(j.patch(patch), json::parse_error&);
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'move' must have string member 'from'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'move' must have string member 'from'", json::parse_error&);
#endif
            }

            SECTION("nonexisting from location (array)")
            {
                json const j = {1, 2, 3};
                json const patch = {{{"op", "move"}, {"path", "/0"}, {"from", "/5"}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.401] array index 5 is out of range", json::out_of_range&);
            }

            SECTION("nonexisting from location (object)")
            {
                json const j = {{"foo", 1}, {"bar", 2}};
                json const patch = {{{"op", "move"}, {"path", "/baz"}, {"from", "/baz"}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.403] key 'baz' not found", json::out_of_range&);
            }
        }

        SECTION("copy")
        {
            SECTION("missing 'path'")
            {
                json const j;
                json const patch = {{{"op", "copy"}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'copy' must have member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'copy' must have member 'path'", json::parse_error&);
#endif
            }

            SECTION("non-string 'path'")
            {
                json const j;
                json const patch = {{{"op", "copy"}, {"path", 1}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'copy' must have string member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'copy' must have string member 'path'", json::parse_error&);
#endif
            }

            SECTION("missing 'from'")
            {
                json const j;
                json const patch = {{{"op", "copy"}, {"path", ""}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'copy' must have member 'from'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'copy' must have member 'from'", json::parse_error&);
#endif
            }

            SECTION("non-string 'from'")
            {
                json const j;
                json const patch = {{{"op", "copy"}, {"path", ""}, {"from", 1}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'copy' must have string member 'from'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'copy' must have string member 'from'", json::parse_error&);
#endif
            }

            SECTION("nonexisting from location (array)")
            {
                json const j = {1, 2, 3};
                json const patch = {{{"op", "copy"}, {"path", "/0"}, {"from", "/5"}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.401] array index 5 is out of range", json::out_of_range&);
            }

            SECTION("nonexisting from location (object)")
            {
                json const j = {{"foo", 1}, {"bar", 2}};
                json const patch = {{{"op", "copy"}, {"path", "/fob"}, {"from", "/baz"}}};
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.out_of_range.403] key 'baz' not found", json::out_of_range&);
            }
        }

        SECTION("test")
        {
            SECTION("missing 'path'")
            {
                json const j;
                json const patch = {{{"op", "test"}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'test' must have member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'test' must have member 'path'", json::parse_error&);
#endif
            }

            SECTION("non-string 'path'")
            {
                json const j;
                json const patch = {{{"op", "test"}, {"path", 1}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'test' must have string member 'path'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'test' must have string member 'path'", json::parse_error&);
#endif
            }

            SECTION("missing 'value'")
            {
                json const j;
                json const patch = {{{"op", "test"}, {"path", ""}}};
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: (/0) operation 'test' must have member 'value'", json::parse_error&);
#else
                CHECK_THROWS_WITH_AS(j.patch(patch), "[json.exception.parse_error.105] parse error: operation 'test' must have member 'value'", json::parse_error&);
#endif
            }
        }
    }

    SECTION("Examples from jsonpatch.com")
    {
        SECTION("Simple Example")
        {

            json const doc = R"(
                {
                  "baz": "qux",
                  "foo": "bar"
                }
            )"_json;

            json const patch = R"(
                [
                  { "op": "replace", "path": "/baz", "value": "boo" },
                  { "op": "add", "path": "/hello", "value": ["world"] },
                  { "op": "remove", "path": "/foo"}
                ]
            )"_json;

            json result = R"(
                {
                   "baz": "boo",
                   "hello": ["world"]
                }
            )"_json;

            CHECK(doc.patch(patch) == result);

            CHECK(doc.patch(json::diff(doc, result)) == result);
        }

        SECTION("Operations")
        {

            json const doc = R"(
                {
                  "biscuits": [
                    {"name":"Digestive"},
                    {"name": "Choco Liebniz"}
                  ]
                }
            )"_json;

            SECTION("add")
            {

                json const patch = R"(
                    [
                        {"op": "add", "path": "/biscuits/1", "value": {"name": "Ginger Nut"}}
                    ]
                )"_json;

                json result = R"(
                    {
                      "biscuits": [
                        {"name": "Digestive"},
                        {"name": "Ginger Nut"},
                        {"name": "Choco Liebniz"}
                      ]
                    }
                )"_json;

                CHECK(doc.patch(patch) == result);

                CHECK(doc.patch(json::diff(doc, result)) == result);
            }

            SECTION("remove")
            {

                json const patch = R"(
                    [
                        {"op": "remove", "path": "/biscuits"}
                    ]
                )"_json;

                json result = R"(
                    {}
                )"_json;

                CHECK(doc.patch(patch) == result);

                CHECK(doc.patch(json::diff(doc, result)) == result);
            }

            SECTION("replace")
            {

                json const patch = R"(
                    [
                        {"op": "replace", "path": "/biscuits/0/name", "value": "Chocolate Digestive"}
                    ]
                )"_json;

                json result = R"(
                    {
                      "biscuits": [
                        {"name": "Chocolate Digestive"},
                        {"name": "Choco Liebniz"}
                      ]
                    }
                )"_json;

                CHECK(doc.patch(patch) == result);

                CHECK(doc.patch(json::diff(doc, result)) == result);
            }

            SECTION("copy")
            {

                json const patch = R"(
                    [
                        {"op": "copy", "from": "/biscuits/0", "path": "/best_biscuit"}
                    ]
                )"_json;

                json result = R"(
                    {
                      "biscuits": [
                        {"name": "Digestive"},
                        {"name": "Choco Liebniz"}
                      ],
                      "best_biscuit": {
                        "name": "Digestive"
                      }
                    }
                )"_json;

                CHECK(doc.patch(patch) == result);

                CHECK(doc.patch(json::diff(doc, result)) == result);
            }

            SECTION("move")
            {

                json const patch = R"(
                    [
                        {"op": "move", "from": "/biscuits", "path": "/cookies"}
                    ]
                )"_json;

                json result = R"(
                    {
                      "cookies": [
                        {"name": "Digestive"},
                        {"name": "Choco Liebniz"}
                      ]
                    }
                )"_json;

                CHECK(doc.patch(patch) == result);

                CHECK(doc.patch(json::diff(doc, result)) == result);
            }

            SECTION("test")
            {

                json patch = R"(
                    [
                        {"op": "test", "path": "/best_biscuit/name", "value": "Choco Liebniz"}
                    ]
                )"_json;

                CHECK_THROWS_AS(doc.patch(patch), json::other_error&);
#if JSON_DIAGNOSTICS
                CHECK_THROWS_WITH_STD_STR(doc.patch(patch), "[json.exception.other_error.501] (/0) unsuccessful: " + patch[0].dump());
#else
                CHECK_THROWS_WITH_STD_STR(doc.patch(patch), "[json.exception.other_error.501] unsuccessful: " + patch[0].dump());
#endif
            }
        }
    }

    SECTION("Examples from bruth.github.io/jsonpatch-js")
    {
        SECTION("add")
        {
            CHECK(R"( {} )"_json.patch(
                      R"( [{"op": "add", "path": "/foo", "value": "bar"}] )"_json
                  ) == R"( {"foo": "bar"} )"_json);

            CHECK(R"( {"foo": [1, 3]} )"_json.patch(
                      R"( [{"op": "add", "path": "/foo", "value": "bar"}] )"_json
                  ) == R"( {"foo": "bar"} )"_json);

            CHECK(R"( {"foo": [{}]} )"_json.patch(
                      R"( [{"op": "add", "path": "/foo/0/bar", "value": "baz"}] )"_json
                  ) == R"( {"foo": [{"bar": "baz"}]} )"_json);
        }

        SECTION("remove")
        {
            CHECK(R"( {"foo": "bar"} )"_json.patch(
                      R"( [{"op": "remove", "path": "/foo"}] )"_json
                  ) == R"( {} )"_json);

            CHECK(R"( {"foo": [1, 2, 3]} )"_json.patch(
                      R"( [{"op": "remove", "path": "/foo/1"}] )"_json
                  ) == R"( {"foo": [1, 3]} )"_json);

            CHECK(R"( {"foo": [{"bar": "baz"}]} )"_json.patch(
                      R"( [{"op": "remove", "path": "/foo/0/bar"}] )"_json
                  ) == R"( {"foo": [{}]} )"_json);
        }

        SECTION("replace")
        {
            CHECK(R"( {"foo": "bar"} )"_json.patch(
                      R"( [{"op": "replace", "path": "/foo", "value": 1}] )"_json
                  ) == R"( {"foo": 1} )"_json);

            CHECK(R"( {"foo": [1, 2, 3]} )"_json.patch(
                      R"( [{"op": "replace", "path": "/foo/1", "value": 4}] )"_json
                  ) == R"( {"foo": [1, 4, 3]} )"_json);

            CHECK(R"( {"foo": [{"bar": "baz"}]} )"_json.patch(
                      R"( [{"op": "replace", "path": "/foo/0/bar", "value": 1}] )"_json
                  ) == R"( {"foo": [{"bar": 1}]} )"_json);
        }

        SECTION("move")
        {
            CHECK(R"( {"foo": [1, 2, 3]} )"_json.patch(
                      R"( [{"op": "move", "from": "/foo", "path": "/bar"}] )"_json
                  ) == R"( {"bar": [1, 2, 3]} )"_json);
        }

        SECTION("copy")
        {
            CHECK(R"( {"foo": [1, 2, 3]} )"_json.patch(
                      R"( [{"op": "copy", "from": "/foo/1", "path": "/bar"}] )"_json
                  ) == R"( {"foo": [1, 2, 3], "bar": 2} )"_json);
        }

        SECTION("copy")
        {
            CHECK_NOTHROW(R"( {"foo": "bar"} )"_json.patch(
                              R"( [{"op": "test", "path": "/foo", "value": "bar"}] )"_json));
        }
    }

    SECTION("Tests from github.com/json-patch/json-patch-tests")
    {
        for (const auto* filename :
                {
                    TEST_DATA_DIRECTORY "/json-patch-tests/spec_tests.json",
                    TEST_DATA_DIRECTORY "/json-patch-tests/tests.json"
                })
        {
            CAPTURE(filename)
            std::ifstream f(filename);
            json const suite = json::parse(f);

            for (const auto& test : suite)
            {
                INFO_WITH_TEMP(test.value("comment", ""));

                if (test.value("disabled", false))
                {
                    continue;
                }

                const auto& doc = test["doc"];
                const auto& patch = test["patch"];

                if (test.count("error") == 0)
                {

                    const auto& expected = test.value("expected", doc);
                    CHECK(doc.patch(patch) == expected);
                }
                else
                {
                    CHECK_THROWS(doc.patch(patch));
                }
            }
        }
    }
}
