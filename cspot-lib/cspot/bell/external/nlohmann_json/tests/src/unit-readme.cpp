

#include "doctest_compatibility.h"

#include <nlohmann/json.hpp>
using nlohmann::json;
#ifdef JSON_TEST_NO_GLOBAL_UDLS
    using namespace nlohmann::literals;
#endif

#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include <iomanip>

DOCTEST_MSVC_SUPPRESS_WARNING_PUSH
DOCTEST_MSVC_SUPPRESS_WARNING(4189)

TEST_CASE("README" * doctest::skip())
{
    {

        auto* old_cout_buffer = std::cout.rdbuf();
        std::ostringstream const new_stream;
        std::cout.rdbuf(new_stream.rdbuf());
        {

            json j;

            j["pi"] = 3.141;

            j["happy"] = true;

            j["name"] = "Niels";

            j["nothing"] = nullptr;

            j["answer"]["everything"] = 42;

            j["list"] = { 1, 0, 2 };

            j["object"] = { {"currency", "USD"}, {"value", 42.99} };

            json const j2 =
            {
                {"pi", 3.141},
                {"happy", true},
                {"name", "Niels"},
                {"nothing", nullptr},
                {
                    "answer", {
                        {"everything", 42}
                    }
                },
                {"list", {1, 0, 2}},
                {
                    "object", {
                        {"currency", "USD"},
                        {"value", 42.99}
                    }
                }
            };
        }

        {

            json const empty_array_implicit = {{}};
            CHECK(empty_array_implicit.is_array());
            json const empty_array_explicit = json::array();
            CHECK(empty_array_explicit.is_array());

            json const empty_object_explicit = json::object();
            CHECK(empty_object_explicit.is_object());

            json array_not_object = json::array({ {"currency", "USD"}, {"value", 42.99} });
            CHECK(array_not_object.is_array());
            CHECK(array_not_object.size() == 2);
            CHECK(array_not_object[0].is_array());
            CHECK(array_not_object[1].is_array());
        }

        {

            json const j = "{ \"happy\": true, \"pi\": 3.141 }"_json;

            auto j2 = R"({
                "happy": true,
                "pi": 3.141
            })"_json;

            auto j3 = json::parse(R"({"happy": true, "pi": 3.141})");

            std::string const s = j.dump();

            std::cout << j.dump(4) << std::endl;

            std::cout << std::setw(2) << j << std::endl;
        }

        {

            json j;
            j.push_back("foo");
            j.push_back(1);
            j.push_back(true);

            bool x = (j == R"(["foo", 1, true])"_json);
            CHECK(x == true);

            for (json::iterator it = j.begin(); it != j.end(); ++it)
            {
                std::cout << *it << '\n';
            }

            for (auto& element : j)
            {
                std::cout << element << '\n';
            }

            const auto tmp = j[0].get<std::string>();
            j[1] = 42;
            bool foo{j.at(2)};
            CHECK(foo == true);

            CHECK(j.size() == 3);
            CHECK_FALSE(j.empty());
            CHECK(j.type() == json::value_t::array);
            j.clear();

            json o;
            o["foo"] = 23;
            o["bar"] = false;
            o["baz"] = 3.141;

            CHECK(o.find("foo") != o.end());
            if (o.find("foo") != o.end())
            {

            }
        }

        {
            std::vector<int> const c_vector {1, 2, 3, 4};
            json const j_vec(c_vector);

            std::deque<float> const c_deque {1.2f, 2.3f, 3.4f, 5.6f};
            json const j_deque(c_deque);

            std::list<bool> const c_list {true, true, false, true};
            json const j_list(c_list);

            std::forward_list<int64_t> const c_flist {12345678909876, 23456789098765, 34567890987654, 45678909876543};
            json const j_flist(c_flist);

            std::array<unsigned long, 4> const c_array {{1, 2, 3, 4}};
            json const j_array(c_array);

            std::set<std::string> const c_set {"one", "two", "three", "four", "one"};
            json const j_set(c_set);

            std::unordered_set<std::string> const c_uset {"one", "two", "three", "four", "one"};
            json const j_uset(c_uset);

            std::multiset<std::string> const c_mset {"one", "two", "one", "four"};
            json const j_mset(c_mset);

            std::unordered_multiset<std::string> const c_umset {"one", "two", "one", "four"};
            json const j_umset(c_umset);

        }

        {
            std::map<std::string, int> const c_map { {"one", 1}, {"two", 2}, {"three", 3} };
            json const j_map(c_map);

            std::unordered_map<const char*, float> const c_umap { {"one", 1.2f}, {"two", 2.3f}, {"three", 3.4f} };
            json const j_umap(c_umap);

            std::multimap<std::string, bool> const c_mmap { {"one", true}, {"two", true}, {"three", false}, {"three", true} };
            json const j_mmap(c_mmap);

            std::unordered_multimap<std::string, bool> const c_ummap { {"one", true}, {"two", true}, {"three", false}, {"three", true} };
            json const j_ummap(c_ummap);

        }

        {

            std::string const s1 = "Hello, world!";
            json const js = s1;
            auto s2 = js.get<std::string>();

            bool const b1 = true;
            json const jb = b1;
            bool b2{jb};
            CHECK(b2 == true);

            int const i = 42;
            json const jn = i;
            double f{jn};
            CHECK(f == 42);

            std::string const vs = js.get<std::string>();
            bool vb = jb.get<bool>();
            CHECK(vb == true);
            int vi = jn.get<int>();
            CHECK(vi == 42);

        }

        {

            json j_original = R"({
                "baz": ["one", "two", "three"],
                "foo": "bar"
            })"_json;

            j_original["/baz/1"_json_pointer];

            json const j_patch = R"([
                { "op": "replace", "path": "/baz", "value": "boo" },
                { "op": "add", "path": "/hello", "value": ["world"] },
                { "op": "remove", "path": "/foo"}
            ])"_json;

            json const j_result = j_original.patch(j_patch);

            auto res = json::diff(j_result, j_original);

        }

        std::cout.rdbuf(old_cout_buffer);
    }
}

DOCTEST_MSVC_SUPPRESS_WARNING_POP
