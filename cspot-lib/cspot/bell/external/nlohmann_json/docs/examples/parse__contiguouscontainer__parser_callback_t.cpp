#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    std::vector<std::uint8_t> text = {'[', '1', ',', '2', ',', '3', ']', '\0'};

    json j_complete = json::parse(text);
    std::cout << std::setw(4) << j_complete << "\n\n";
}
