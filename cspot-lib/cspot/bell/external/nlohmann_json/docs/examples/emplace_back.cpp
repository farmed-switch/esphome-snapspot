#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json array = {1, 2, 3, 4, 5};
    json null;

    std::cout << array << '\n';
    std::cout << null << '\n';

    array.emplace_back(6);
    null.emplace_back("first");
    null.emplace_back(3, "second");

    std::cout << array << '\n';
    std::cout << null << '\n';
}
