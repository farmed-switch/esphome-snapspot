#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json object = {{"one", 1}, {"two", 2}};
    json null;

    std::cout << object << '\n';
    std::cout << null << '\n';

    object.push_back({"three", 3});
    object += {"four", 4};
    null.push_back({"five", 5});

    std::cout << object << '\n';
    std::cout << null << '\n';

}
