#include <compare>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const char* to_string(const std::partial_ordering& po)
{
    if (std::is_lt(po))
    {
        return "less";
    }
    else if (std::is_gt(po))
    {
        return "greater";
    }
    else if (std::is_eq(po))
    {
        return "equivalent";
    }
    return "unordered";
}

int main()
{
    using float_limits = std::numeric_limits<json::number_float_t>;
    constexpr auto nan = float_limits::quiet_NaN();

    json boolean = false;
    json number = 17;
    json string = "17";

    std::cout << std::boolalpha << std::fixed;
    std::cout << boolean << " <=> " << true << " := " << to_string(boolean <=> true) << '\n';
    std::cout << number << " <=> " << 17.0 << " := " << to_string(number <=> 17.0) << '\n';
    std::cout << number << " <=> " << nan << " := " << to_string(number <=> nan) << '\n';
    std::cout << string << " <=> " << 17 << " := " << to_string(string <=> 17) << '\n';
}
