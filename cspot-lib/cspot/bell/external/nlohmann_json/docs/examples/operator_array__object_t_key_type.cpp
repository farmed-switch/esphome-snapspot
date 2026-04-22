#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json object =
    {
        {"one", 1}, {"two", 2}, {"three", 2.9}
    };

    std::cout << object["two"] << "\n\n";

    object["three"] = 3;

    std::cout << std::setw(4) << object << "\n\n";

    object["four"];

    object["five"]["really"]["nested"] = true;

    std::cout << std::setw(4) << object << '\n';
}
