#include <iostream>
#include <nlohmann/json.hpp>
#include <iomanip>

using json = nlohmann::json;
using namespace nlohmann::literals;

int main()
{

    json document = R"({
                "title": "Goodbye!",
                "author": {
                    "givenName": "John",
                    "familyName": "Doe"
                },
                "tags": [
                    "example",
                    "sample"
                ],
                "content": "This will be unchanged"
            })"_json;

    json patch = R"({
                "title": "Hello!",
                "phoneNumber": "+01-123-456-7890",
                "author": {
                    "familyName": null
                },
                "tags": [
                    "example"
                ]
            })"_json;

    document.merge_patch(patch);

    std::cout << std::setw(4) << document << std::endl;
}
