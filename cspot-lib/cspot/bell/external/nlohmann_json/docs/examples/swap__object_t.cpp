#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json value = { {"translation", {{"one", "eins"}, {"two", "zwei"}}} };

    json::object_t object = {{"cow", "Kuh"}, {"dog", "Hund"}};

    value["translation"].swap(object);

    std::cout << "value = " << value << '\n';
    std::cout << "object = " << object << '\n';
}
