#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json::json_pointer ptr("/foo");
    std::cout << "\"" << ptr << "\"\n";

    ptr /= json::json_pointer("/bar/baz");
    std::cout << "\"" << ptr << "\"\n";

    ptr /= "fob";
    std::cout << "\"" << ptr << "\"\n";

    ptr /= 42;
    std::cout << "\"" << ptr << "\"" << std::endl;
}
