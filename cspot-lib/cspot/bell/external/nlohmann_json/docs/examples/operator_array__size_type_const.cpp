#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    const json array = {"first", "2nd", "third", "fourth"};

    std::cout << array.at(2) << '\n';
}
