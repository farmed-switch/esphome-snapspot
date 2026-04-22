#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json value = { "the good", "the bad", "the ugly" };

    json::string_t string = "the fast";

    value[1].swap(string);

    std::cout << "value = " << value << '\n';
    std::cout << "string = " << string << '\n';
}
