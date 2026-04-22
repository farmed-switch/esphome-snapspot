#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace nlohmann::literals;

int main()
{

    json doc = R"(
        {
          "baz": "qux",
          "foo": "bar"
        }
    )"_json;

    json patch = R"(
        [
          { "op": "replace", "path": "/baz", "value": "boo" },
          { "op": "add", "path": "/hello", "value": ["world"] },
          { "op": "remove", "path": "/foo"}
        ]
    )"_json;

    std::cout << "Before\n" << std::setw(4) << doc << std::endl;

    doc.patch_inplace(patch);

    std::cout << "\nAfter\n" << std::setw(4) << doc << std::endl;
}
