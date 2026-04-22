#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json a = 23;
    json b = 42;

    b = a;

    std::cout << a << '\n';
    std::cout << b << '\n';
}
