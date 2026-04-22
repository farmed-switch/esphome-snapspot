#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json::json_pointer ptr("/foo/bar/baz");

    std::cout << ptr << std::endl;
}
