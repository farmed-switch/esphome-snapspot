#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using std::to_string;

int main()
{

    json j = {{"one", 1}, {"two", 2}};
    int i = 42;

    auto j_str = to_string(j);
    auto i_str = to_string(i);

    std::cout << j_str << "\n\n"
              << i_str << std::endl;
}
