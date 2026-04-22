#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json j1;

    json j2(nullptr);

    std::cout << j1 << '\n' << j2 << '\n';
}
