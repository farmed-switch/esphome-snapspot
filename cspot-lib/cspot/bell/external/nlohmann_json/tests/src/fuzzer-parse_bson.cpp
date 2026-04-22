

#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    try
    {

        std::vector<uint8_t> const vec1(data, data + size);
        json const j1 = json::from_bson(vec1);

        if (j1.is_discarded())
        {
            return 0;
        }

        try
        {

            std::vector<uint8_t> const vec2 = json::to_bson(j1);

            json const j2 = json::from_bson(vec2);

            assert(json::to_bson(j2) == vec2);
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
