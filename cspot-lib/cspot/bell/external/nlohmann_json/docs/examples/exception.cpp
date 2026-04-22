#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
    try
    {

        json j = {{"foo", "bar"}};
        json k = j.at("non-existing");
    }
    catch (json::exception& e)
    {

        std::cout << "message: " << e.what() << '\n'
                  << "exception id: " << e.id << std::endl;
    }
}
