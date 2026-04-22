#pragma once

#include <type_traits>

#include <nlohmann/detail/abi_macros.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN
namespace detail
{

struct json_default_base {};

template<class T>
using json_base_class = typename std::conditional <
                        std::is_same<T, void>::value,
                        json_default_base,
                        T
                        >::type;

}
NLOHMANN_JSON_NAMESPACE_END
