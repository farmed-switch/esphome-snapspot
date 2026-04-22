#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json j_object = {{"one", 1}, {"two", 2}};
    json j_array = {1, 2, 4, 8, 16};
    json j_string = "Hellö 😀!";

    std::cout << "objects:" << '\n'
              << j_object.dump() << "\n\n"
              << j_object.dump(-1) << "\n\n"
              << j_object.dump(0) << "\n\n"
              << j_object.dump(4) << "\n\n"
              << j_object.dump(1, '\t') << "\n\n";

    std::cout << "arrays:" << '\n'
              << j_array.dump() << "\n\n"
              << j_array.dump(-1) << "\n\n"
              << j_array.dump(0) << "\n\n"
              << j_array.dump(4) << "\n\n"
              << j_array.dump(1, '\t') << "\n\n";

    std::cout << "strings:" << '\n'
              << j_string.dump() << '\n'
              << j_string.dump(-1, ' ', true) << '\n';

    json j_invalid = "ä\xA9ü";
    try
    {
        std::cout << j_invalid.dump() << std::endl;
    }
    catch (json::type_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    std::cout << "string with replaced invalid characters: "
              << j_invalid.dump(-1, ' ', false, json::error_handler_t::replace)
              << "\nstring with ignored invalid characters: "
              << j_invalid.dump(-1, ' ', false, json::error_handler_t::ignore)
              << '\n';
}
