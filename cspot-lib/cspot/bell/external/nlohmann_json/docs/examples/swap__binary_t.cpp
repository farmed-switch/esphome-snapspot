#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json value = json::binary({1, 2, 3});

    json::binary_t binary = {{4, 5, 6}};

    value.swap(binary);

    std::cout << "value = " << value << '\n';
    std::cout << "binary = " << json(binary) << '\n';
}
