#include <iostream>
#include <nlohmann/json.hpp>

template<typename Map>
void output(const char* prefix, const Map& m)
{
    std::cout << prefix << " = { ";
    for (auto& element : m)
    {
        std::cout << element.first << ":" << element.second << ' ';
    }
    std::cout << "}" << std::endl;
}

int main()
{

    nlohmann::ordered_map<std::string, std::string> m_ordered;
    m_ordered["one"] = "eins";
    m_ordered["two"] = "zwei";
    m_ordered["three"] = "drei";

    std::map<std::string, std::string> m_std;
    m_std["one"] = "eins";
    m_std["two"] = "zwei";
    m_std["three"] = "drei";

    output("m_ordered", m_ordered);
    output("m_std", m_std);

    m_ordered.erase("one");
    m_ordered["one"] = "eins";

    m_std.erase("one");
    m_std["one"] = "eins";

    output("m_ordered", m_ordered);
    output("m_std", m_std);
}
