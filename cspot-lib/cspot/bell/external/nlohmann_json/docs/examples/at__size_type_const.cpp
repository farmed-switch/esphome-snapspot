#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    const json array = {"first", "2nd", "third", "fourth"};

    std::cout << array.at(2) << '\n';

    try
    {

        const json str = "I am a string";
        std::cout << str.at(0) << '\n';
    }
    catch (json::type_error& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {

        std::cout << array.at(5) << '\n';
    }
    catch (json::out_of_range& e)
    {
        std::cout << e.what() << '\n';
    }
}
