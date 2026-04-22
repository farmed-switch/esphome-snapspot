#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json value = "Hello";
    json array_0 = json(0, value);
    json array_1 = json(1, value);
    json array_5 = json(5, value);

    std::cout << array_0 << '\n';
    std::cout << array_1 << '\n';
    std::cout << array_5 << '\n';
}
