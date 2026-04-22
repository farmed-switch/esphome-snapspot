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

    json patched_doc = doc.patch(patch);

    std::cout << std::setw(4) << doc << "\n\n"
              << std::setw(4) << patched_doc << std::endl;
}
