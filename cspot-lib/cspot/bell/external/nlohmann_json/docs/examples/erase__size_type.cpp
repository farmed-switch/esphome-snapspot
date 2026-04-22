#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json j_array = {0, 1, 2, 3, 4, 5};

    j_array.erase(2);

    std::cout << j_array << '\n';
}
