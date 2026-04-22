#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json j =
    {
        {"integer", 1},
        {"floating", 42.23},
        {"string", "hello world"},
        {"boolean", true},
        {"object", {{"key1", 1}, {"key2", 2}}},
        {"array", {1, 2, 3}}
    };

    int v_integer = j.value("integer", 0);
    double v_floating = j.value("floating", 47.11);

    std::string v_string = j.value("nonexisting", "oops");
    bool v_boolean = j.value("nonexisting", false);

    std::cout << std::boolalpha << v_integer << " " << v_floating
              << " " << v_string << " " << v_boolean << "\n";
}
