#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace nlohmann::literals;

int main()
{

    json j =
    {
        {"number", 1}, {"string", "foo"}, {"array", {1, 2}}
    };

    std::cout << j["/number"_json_pointer] << '\n';

    std::cout << j["/string"_json_pointer] << '\n';

    std::cout << j["/array"_json_pointer] << '\n';

    std::cout << j["/array/1"_json_pointer] << '\n';

    j["/string"_json_pointer] = "bar";

    std::cout << j["string"] << '\n';

    j["/boolean"_json_pointer] = true;

    std::cout << j << '\n';

    j["/array/1"_json_pointer] = 21;

    j["/array/4"_json_pointer] = 44;

    std::cout << j["array"] << '\n';

    j["/array/-"_json_pointer] = 55;

    std::cout << j["array"] << '\n';
}
