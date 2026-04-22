#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace nlohmann::literals;

int main()
{

    json o1 = R"( {"color": "red", "price": 17.99, "names": {"de": "Flugzeug"}} )"_json;
    json o2 = R"( {"color": "blue", "speed": 100, "names": {"en": "plane"}} )"_json;
    json o3 = o1;

    o1.update(o2.begin(), o2.end());

    o3.update(o2.begin(), o2.end(), true);

    std::cout << std::setw(2) << o1 << '\n';
    std::cout << std::setw(2) << o3 << '\n';
}
