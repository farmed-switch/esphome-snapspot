#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json v = {1, 2, 3, 4};

    json v2 = {"one", "two", "three", "four"};

    auto new_pos = v.insert(v.end(), v2.begin(), v2.end());

    std::cout << *new_pos << '\n';
    std::cout << v << '\n';
}
