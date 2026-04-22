#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
    try
    {

        json j = "string";
        json::iterator it = j.begin();
        auto k = it.key();
    }
    catch (json::invalid_iterator& e)
    {

        std::cout << "message: " << e.what() << '\n'
                  << "exception id: " << e.id << std::endl;
    }
}
