#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json j_object = {{"one", 1}, {"two", 2}};

    auto count_two = j_object.count("two");
    auto count_three = j_object.count("three");

    std::cout << "number of elements with key \"two\": " << count_two << '\n';
    std::cout << "number of elements with key \"three\": " << count_three << '\n';
}
