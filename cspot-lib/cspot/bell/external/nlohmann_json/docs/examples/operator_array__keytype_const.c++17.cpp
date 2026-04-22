#include <iostream>
#include <string_view>
#include <nlohmann/json.hpp>

using namespace std::string_view_literals;
using json = nlohmann::json;

int main()
{

    const json object =
    {
        {"one", 1}, {"two", 2}, {"three", 2.9}
    };

    std::cout << object["two"sv] << '\n';
}
