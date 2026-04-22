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

    std::cout << j.at("/number"_json_pointer) << '\n';

    std::cout << j.at("/string"_json_pointer) << '\n';

    std::cout << j.at("/array"_json_pointer) << '\n';

    std::cout << j.at("/array/1"_json_pointer) << '\n';

    try
    {

        json::const_reference ref = j.at("/array/one"_json_pointer);
    }
    catch (json::parse_error& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {

        json::const_reference ref = j.at("/array/4"_json_pointer);
    }
    catch (json::out_of_range& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {

        json::const_reference ref = j.at("/array/-"_json_pointer);
    }
    catch (json::out_of_range& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {

        json::const_reference ref = j.at("/foo"_json_pointer);
    }
    catch (json::out_of_range& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {

        json::const_reference ref = j.at("/number/foo"_json_pointer);
    }
    catch (json::out_of_range& e)
    {
        std::cout << e.what() << '\n';
    }
}
