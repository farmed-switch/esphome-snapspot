#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
    try
    {

        json j = "string";
        j.push_back("another string");
    }
    catch (json::type_error& e)
    {

        std::cout << "message: " << e.what() << '\n'
                  << "exception id: " << e.id << std::endl;
    }
}
