#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    const json object =
    {
        {"one", 1}, {"two", 2}, {"three", 2.9}
    };

    std::cout << object["two"] << '\n';
}
