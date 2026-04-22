#include <iostream>
#include <string_view>
#include <nlohmann/json.hpp>

using namespace std::string_view_literals;
using json = nlohmann::json;
using namespace nlohmann::literals;

int main()
{

    json j_object = R"( {"key": "value"} )"_json;
    json j_array = R"( [1, 2, 3] )"_json;

    std::cout << std::boolalpha <<
              "j_object contains 'key': " << j_object.contains("key"sv) << '\n' <<
              "j_object contains 'another': " << j_object.contains("another"sv) << '\n' <<
              "j_array contains 'key': " << j_array.contains("key"sv) << std::endl;
}
