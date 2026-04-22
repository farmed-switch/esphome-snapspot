#include <iostream>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    std::stringstream ss;
    ss << R"({
        "number": 23,
        "string": "Hello, world!",
        "array": [1, 2, 3, 4, 5],
        "boolean": false,
        "null": null
    })";

    json j;
    ss >> j;

    std::cout << std::setw(2) << j << '\n';
}
