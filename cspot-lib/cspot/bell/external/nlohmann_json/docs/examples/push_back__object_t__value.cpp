#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json object = {{"one", 1}, {"two", 2}};
    json null;

    std::cout << object << '\n';
    std::cout << null << '\n';

    object.push_back(json::object_t::value_type("three", 3));
    object += json::object_t::value_type("four", 4);
    null += json::object_t::value_type("A", "a");
    null += json::object_t::value_type("B", "b");

    std::cout << object << '\n';
    std::cout << null << '\n';
}
