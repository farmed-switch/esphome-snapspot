#include <iostream>
#include <string_view>
#include <nlohmann/json.hpp>

using namespace std::string_view_literals;
using json = nlohmann::json;

int main()
{

    const json object =
    {
        {"the good", "il buono"},
        {"the bad", "il cattivo"},
        {"the ugly", "il brutto"}
    };

    std::cout << object.at("the ugly"sv) << '\n';

    try
    {

        const json str = "I am a string";
        std::cout << str.at("the good"sv) << '\n';
    }
    catch (json::type_error& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {

        std::cout << object.at("the fast"sv) << '\n';
    }
    catch (json::out_of_range)
    {
        std::cout << "out of range" << '\n';
    }
}
