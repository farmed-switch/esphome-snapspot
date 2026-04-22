#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json array = {"first", "2nd", "third", "fourth"};

    std::cout << array.at(2) << '\n';

    array.at(1) = "second";

    std::cout << array << '\n';

    try
    {

        json str = "I am a string";
        str.at(0) = "Another string";
    }
    catch (json::type_error& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {

        array.at(5) = "sixth";
    }
    catch (json::out_of_range& e)
    {
        std::cout << e.what() << '\n';
    }
}
