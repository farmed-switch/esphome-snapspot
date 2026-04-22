#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace nlohmann::literals;

void print_byte(uint8_t byte)
{
    if (32 < byte and byte < 128)
    {
        std::cout << (char)byte;
    }
    else
    {
        std::cout << (int)byte;
    }
}

int main()
{

    json j = R"({"compact": true, "schema": false})"_json;

    std::vector<std::uint8_t> v = json::to_bjdata(j);

    for (auto& byte : v)
    {
        print_byte(byte);
    }
    std::cout << std::endl;

    json array = {1, 2, 3, 4, 5, 6, 7, 8};

    std::vector<std::uint8_t> v_array = json::to_bjdata(array);

    std::vector<std::uint8_t> v_array_size = json::to_bjdata(array, true);

    std::vector<std::uint8_t> v_array_size_and_type = json::to_bjdata(array, true, true);

    for (auto& byte : v_array)
    {
        print_byte(byte);
    }
    std::cout << std::endl;

    for (auto& byte : v_array_size)
    {
        print_byte(byte);
    }
    std::cout << std::endl;

    for (auto& byte : v_array_size_and_type)
    {
        print_byte(byte);
    }
    std::cout << std::endl;
}
