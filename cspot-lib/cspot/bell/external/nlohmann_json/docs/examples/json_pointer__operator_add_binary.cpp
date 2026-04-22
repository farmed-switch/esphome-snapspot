#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json::json_pointer ptr("/foo");

    std::cout << "\"" << ptr / json::json_pointer("/bar/baz") << "\"\n";

    std::cout << "\"" << ptr / "fob" << "\"\n";

    std::cout << "\"" << ptr / 42 << "\"" << std::endl;
}
