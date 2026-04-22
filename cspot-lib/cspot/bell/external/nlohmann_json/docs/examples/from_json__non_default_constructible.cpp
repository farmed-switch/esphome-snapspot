#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ns
{

struct person
{
    person(std::string n, std::string a, int aa)
        : name(std::move(n)), address(std::move(a)), age(aa)
    {}

    std::string name;
    std::string address;
    int age;
};
}

namespace nlohmann
{
template <>
struct adl_serializer<ns::person>
{
    static ns::person from_json(const json& j)
    {
        return {j.at("name"), j.at("address"), j.at("age")};
    }

    static void to_json(json& j, ns::person p)
    {
        j["name"] = p.name;
        j["address"] = p.address;
        j["age"] = p.age;
    }
};
}

int main()
{
    json j;
    j["name"] = "Ned Flanders";
    j["address"] = "744 Evergreen Terrace";
    j["age"] = 60;

    auto p = j.get<ns::person>();

    std::cout << p.name << " (" << p.age << ") lives in " << p.address << std::endl;
}
