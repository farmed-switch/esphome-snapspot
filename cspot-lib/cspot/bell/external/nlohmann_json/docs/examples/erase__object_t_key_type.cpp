#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json j_object = {{"one", 1}, {"two", 2}};

    auto count_one = j_object.erase("one");
    auto count_three = j_object.erase("three");

    std::cout << j_object << '\n';
    std::cout << count_one << " " << count_three << '\n';
}
