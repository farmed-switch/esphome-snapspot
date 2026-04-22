

#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    try
    {

        std::vector<uint8_t> const vec1(data, data + size);
        json const j1 = json::from_bjdata(vec1);

        try
        {

            std::vector<uint8_t> const vec2 = json::to_bjdata(j1, false, false);

            std::vector<uint8_t> const vec3 = json::to_bjdata(j1, true, false);

            std::vector<uint8_t> const vec4 = json::to_bjdata(j1, true, true);

            json const j2 = json::from_bjdata(vec2);
            json const j3 = json::from_bjdata(vec3);
            json const j4 = json::from_bjdata(vec4);

            assert(json::to_bjdata(j2, false, false) == vec2);
            assert(json::to_bjdata(j3, true, false) == vec3);
            assert(json::to_bjdata(j4, true, true) == vec4);
        }
        catch (const json::parse_error&)
        {

            assert(false);
        }
    }
    catch (const json::parse_error&)
    {

    }
    catch (const json::type_error&)
    {

    }
    catch (const json::out_of_range&)
    {

    }

    return 0;
}
