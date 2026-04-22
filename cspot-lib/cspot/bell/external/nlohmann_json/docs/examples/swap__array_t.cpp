#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json value = {{"array", {1, 2, 3, 4}}};

    json::array_t array = {"Snap", "Crackle", "Pop"};

    value["array"].swap(array);

    std::cout << "value = " << value << '\n';
    std::cout << "array = " << array << '\n';
}
