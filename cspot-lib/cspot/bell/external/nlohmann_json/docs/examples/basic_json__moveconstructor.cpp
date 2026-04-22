#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json a = 23;

    json b(std::move(a));

    std::cout << a << '\n';
    std::cout << b << '\n';
}
