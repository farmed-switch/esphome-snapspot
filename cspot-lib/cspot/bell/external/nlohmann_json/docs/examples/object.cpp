#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json j_no_init_list = json::object();
    json j_empty_init_list = json::object({});
    json j_list_of_pairs = json::object({ {"one", 1}, {"two", 2} });

    std::cout << j_no_init_list << '\n';
    std::cout << j_empty_init_list << '\n';
    std::cout << j_list_of_pairs << '\n';

    try
    {

        json j_invalid_object = json::object({{ "one", 1, 2 }});
    }
    catch (json::type_error& e)
    {
        std::cout << e.what() << '\n';
    }
}
