#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json v = {1, 2, 3, 4};

    auto new_pos = v.insert(v.begin() + 2, 7, 7);

    std::cout << *new_pos << '\n';
    std::cout << v << '\n';
}
