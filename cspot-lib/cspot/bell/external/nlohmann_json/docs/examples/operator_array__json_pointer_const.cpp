#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace nlohmann::literals;

int main()
{

    const json j =
    {
        {"number", 1}, {"string", "foo"}, {"array", {1, 2}}
    };

    std::cout << j["/number"_json_pointer] << '\n';

    std::cout << j["/string"_json_pointer] << '\n';

    std::cout << j["/array"_json_pointer] << '\n';

    std::cout << j["/array/1"_json_pointer] << '\n';
}
