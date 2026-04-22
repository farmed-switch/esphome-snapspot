

#pragma once

#include <nlohmann/detail/abi_macros.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN
namespace detail
{

template<typename StringType>
inline void replace_substring(StringType& s, const StringType& f,
                              const StringType& t)
{
    JSON_ASSERT(!f.empty());
    for (auto pos = s.find(f);
            pos != StringType::npos;
            s.replace(pos, f.size(), t),
            pos = s.find(f, pos + t.size()))
    {}
}

template<typename StringType>
inline StringType escape(StringType s)
{
    replace_substring(s, StringType{"~"}, StringType{"~0"});
    replace_substring(s, StringType{"/"}, StringType{"~1"});
    return s;
}

template<typename StringType>
static void unescape(StringType& s)
{
    replace_substring(s, StringType{"~1"}, StringType{"/"});
    replace_substring(s, StringType{"~0"}, StringType{"~"});
}

}
NLOHMANN_JSON_NAMESPACE_END
