#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    std::vector<std::uint8_t> vec = {0xCA, 0xFE, 0xBA, 0xBE};

    json j = json::binary(vec, 42);

    std::cout << "type: " << j.type_name() << ", subtype: " << j.get_binary().subtype() << std::endl;
}
