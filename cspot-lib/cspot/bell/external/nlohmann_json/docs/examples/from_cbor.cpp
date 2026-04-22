#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    std::vector<std::uint8_t> v = {0xa2, 0x67, 0x63, 0x6f, 0x6d, 0x70, 0x61, 0x63,
                                   0x74, 0xf5, 0x66, 0x73, 0x63, 0x68, 0x65, 0x6d,
                                   0x61, 0x00
                                  };

    json j = json::from_cbor(v);

    std::cout << std::setw(2) << j << std::endl;
}
