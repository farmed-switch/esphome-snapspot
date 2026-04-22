#include <iostream>
#include <nlohmann/json.hpp>

using json = NLOHMANN_JSON_NAMESPACE::json;

#define Q(x) #x
#define QUOTE(x) Q(x)

int main()
{
    std::cout << QUOTE(NLOHMANN_JSON_NAMESPACE) << std::endl;
}
