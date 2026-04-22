#include <iostream>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>

#define Q(x) #x
#define QUOTE(x) Q(x)

int main()
{
    std::cout << QUOTE(NLOHMANN_JSON_NAMESPACE) << std::endl;
}
