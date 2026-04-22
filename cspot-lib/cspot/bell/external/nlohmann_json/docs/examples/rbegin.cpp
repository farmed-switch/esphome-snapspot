#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json array = {1, 2, 3, 4, 5};

    json::reverse_iterator it = array.rbegin();

    std::cout << *it << '\n';
}
