#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    std::vector<std::uint8_t> input = {'[', '1', ',', '2', ',', '3', ']', 'o', 't', 'h', 'e', 'r'};

    json j_complete = json::parse(input.begin(), input.begin() + 7);
    std::cout << std::setw(4) << j_complete << "\n\n";
}
