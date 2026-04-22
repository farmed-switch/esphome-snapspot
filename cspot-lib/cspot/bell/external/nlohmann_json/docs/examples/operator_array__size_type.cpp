#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json array = {1, 2, 3, 4, 5};

    std::cout << array[3] << '\n';

    array[array.size() - 1] = 6;

    std::cout << array << '\n';

    array[10] = 11;

    std::cout << array << '\n';
}
