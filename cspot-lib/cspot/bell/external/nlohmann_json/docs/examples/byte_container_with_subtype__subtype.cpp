#include <iostream>
#include <nlohmann/json.hpp>

using byte_container_with_subtype = nlohmann::byte_container_with_subtype<std::vector<std::uint8_t>>;

int main()
{
    std::vector<std::uint8_t> bytes = {{0xca, 0xfe, 0xba, 0xbe}};

    auto c1 = byte_container_with_subtype(bytes);

    auto c2 = byte_container_with_subtype(bytes, 42);

    std::cout << "c1.subtype() = " << c1.subtype()
              << "\nc2.subtype() = " << c2.subtype() << std::endl;

    assert(c1.subtype() == static_cast<byte_container_with_subtype::subtype_type>(-1));
}
