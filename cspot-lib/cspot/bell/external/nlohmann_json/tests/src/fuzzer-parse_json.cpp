

#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    try
    {

        json const j1 = json::parse(data, data + size);

        try
        {

            std::string const s1 = j1.dump();

            json const j2 = json::parse(s1);

            std::string const s2 = j2.dump();

            assert(s1 == s2);
        }
        catch (const json::parse_error&)
        {

            assert(false);
        }
    }
    catch (const json::parse_error&)
    {

    }
    catch (const json::out_of_range&)
    {

    }

    return 0;
}
