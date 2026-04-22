#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json array = {1, 2, 3};
    json object = {{"A", "a"}, {"B", "b"}};
    json number = 17;
    json string = "foo";
    json null;

    std::cout << std::boolalpha;
    std::cout << array << " != nullptr " << (array != nullptr) << '\n';
    std::cout << object << " != nullptr " << (object != nullptr) << '\n';
    std::cout << number << " != nullptr " << (number != nullptr) << '\n';
    std::cout << string << " != nullptr " << (string != nullptr) << '\n';
    std::cout << null << " != nullptr " << (null != nullptr) << '\n';
}
