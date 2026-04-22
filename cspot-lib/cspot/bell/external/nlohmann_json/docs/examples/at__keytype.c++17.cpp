#include <iostream>
#include <string_view>
#include <nlohmann/json.hpp>

using namespace std::string_view_literals;
using json = nlohmann::json;

int main()
{

    json object =
    {
        {"the good", "il buono"},
        {"the bad", "il cattivo"},
        {"the ugly", "il brutto"}
    };

    std::cout << object.at("the ugly"sv) << '\n';

    object.at("the bad"sv) = "il cattivo";

    std::cout << object << '\n';

    try
    {

        json str = "I am a string";
        str.at("the good"sv) = "Another string";
    }
    catch (json::type_error& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {

        object.at("the fast"sv) = "il rapido";
    }
    catch (json::out_of_range& e)
    {
        std::cout << e.what() << '\n';
    }
}
