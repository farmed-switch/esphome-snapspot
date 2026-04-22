#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json object =
    {
        {"the good", "il buono"},
        {"the bad", "il cattivo"},
        {"the ugly", "il brutto"}
    };

    std::cout << object.at("the ugly") << '\n';

    object.at("the bad") = "il cattivo";

    std::cout << object << '\n';

    try
    {

        json str = "I am a string";
        str.at("the good") = "Another string";
    }
    catch (json::type_error& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {

        object.at("the fast") = "il rapido";
    }
    catch (json::out_of_range& e)
    {
        std::cout << e.what() << '\n';
    }
}
