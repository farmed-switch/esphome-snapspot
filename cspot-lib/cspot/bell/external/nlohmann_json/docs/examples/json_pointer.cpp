#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{

    json::json_pointer p1;
    json::json_pointer p2("");
    json::json_pointer p3("/");
    json::json_pointer p4("//");
    json::json_pointer p5("/foo/bar");
    json::json_pointer p6("/foo/bar/-");
    json::json_pointer p7("/foo/~0");
    json::json_pointer p8("/foo/~1");

    try
    {
        json::json_pointer p9("foo");
    }
    catch (json::parse_error& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {
        json::json_pointer p10("/foo/~");
    }
    catch (json::parse_error& e)
    {
        std::cout << e.what() << '\n';
    }

    try
    {
        json::json_pointer p11("/foo/~3");
    }
    catch (json::parse_error& e)
    {
        std::cout << e.what() << '\n';
    }
}
