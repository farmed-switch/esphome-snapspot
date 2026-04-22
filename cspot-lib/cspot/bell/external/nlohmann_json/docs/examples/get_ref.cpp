#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json value = 17;

    auto r1 = value.get_ref<const json::number_integer_t&>();
    auto r2 = value.get_ref<json::number_integer_t&>();

    std::cout << r1 << ' ' << r2 << '\n';

    try
    {
        auto r3 = value.get_ref<json::number_float_t&>();
    }
    catch (json::type_error& ex)
    {
        std::cout << ex.what() << '\n';
    }
}
