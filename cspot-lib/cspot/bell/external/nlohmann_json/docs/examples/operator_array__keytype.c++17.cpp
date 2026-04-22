#include <iostream>
#include <iomanip>
#include <string_view>
#include <nlohmann/json.hpp>

using namespace std::string_view_literals;
using json = nlohmann::json;

int main()
{

    json object =
    {
        {"one", 1}, {"two", 2}, {"three", 2.9}
    };

    std::cout << object["two"sv] << "\n\n";

    object["three"sv] = 3;

    std::cout << std::setw(4) << object << "\n\n";

    object["four"sv];

    object["five"sv]["really"sv]["nested"sv] = true;

    std::cout << std::setw(4) << object << '\n';
}
