#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json j1 = {{"one", "eins"}, {"two", "zwei"}};
    json j2 = {{"eleven", "elf"}, {"seventeen", "siebzehn"}};

    std::cout << j1 << '\n';
    std::cout << j2 << '\n';

    j1.insert(j2.begin(), j2.end());

    std::cout << j1 << '\n';
}
