#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json object = {{"one", 1}, {"two", 2}};
    json null;

    std::cout << object << '\n';
    std::cout << null << '\n';

    auto res1 = object.emplace("three", 3);
    null.emplace("A", "a");
    null.emplace("B", "b");

    auto res2 = null.emplace("B", "c");

    std::cout << object << '\n';
    std::cout << *res1.first << " " << std::boolalpha << res1.second << '\n';

    std::cout << null << '\n';
    std::cout << *res2.first << " " << std::boolalpha << res2.second << '\n';
}
