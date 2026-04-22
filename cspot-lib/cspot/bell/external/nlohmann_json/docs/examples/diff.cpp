#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace nlohmann::literals;

int main()
{

    json source = R"(
        {
            "baz": "qux",
            "foo": "bar"
        }
    )"_json;

    json target = R"(
        {
            "baz": "boo",
            "hello": [
                "world"
            ]
        }
    )"_json;

    json patch = json::diff(source, target);

    json patched_source = source.patch(patch);

    std::cout << std::setw(4) << patch << "\n\n"
              << std::setw(4) << patched_source << std::endl;
}
