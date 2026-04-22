#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json array = {1, 2, 3, 4, 5};
    json null;

    std::cout << array << '\n';
    std::cout << null << '\n';

    array.push_back(6);
    array += 7;
    null += "first";
    null += "second";

    std::cout << array << '\n';
    std::cout << null << '\n';
}
