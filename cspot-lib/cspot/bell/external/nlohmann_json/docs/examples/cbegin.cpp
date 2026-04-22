#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    const json array = {1, 2, 3, 4, 5};

    json::const_iterator it = array.cbegin();

    std::cout << *it << '\n';
}
