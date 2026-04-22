#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json j_boolean = true;
    json j_number_integer = 17;
    json j_number_float = 23.42;
    json j_object = {{"one", 1}, {"two", 2}};
    json j_array = {1, 2, 4, 8, 16};
    json j_string = "Hello, world";

    j_boolean.erase(j_boolean.begin());
    j_number_integer.erase(j_number_integer.begin());
    j_number_float.erase(j_number_float.begin());
    j_object.erase(j_object.find("two"));
    j_array.erase(j_array.begin() + 2);
    j_string.erase(j_string.begin());

    std::cout << j_boolean << '\n';
    std::cout << j_number_integer << '\n';
    std::cout << j_number_float << '\n';
    std::cout << j_object << '\n';
    std::cout << j_array << '\n';
    std::cout << j_string << '\n';
}
