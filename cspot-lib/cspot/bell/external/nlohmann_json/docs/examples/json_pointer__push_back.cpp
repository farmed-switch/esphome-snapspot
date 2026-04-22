#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json::json_pointer ptr;
    std::cout << "\"" << ptr << "\"\n";

    ptr.push_back("foo");
    std::cout << "\"" << ptr << "\"\n";

    ptr.push_back("0");
    std::cout << "\"" << ptr << "\"\n";

    ptr.push_back("bar");
    std::cout << "\"" << ptr << "\"\n";
}
