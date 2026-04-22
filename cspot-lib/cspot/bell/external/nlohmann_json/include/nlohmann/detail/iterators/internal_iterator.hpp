

#pragma once

#include <nlohmann/detail/abi_macros.hpp>
#include <nlohmann/detail/iterators/primitive_iterator.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN
namespace detail
{

template<typename BasicJsonType> struct internal_iterator
{

    typename BasicJsonType::object_t::iterator object_iterator {};

    typename BasicJsonType::array_t::iterator array_iterator {};

    primitive_iterator_t primitive_iterator {};
};

}
NLOHMANN_JSON_NAMESPACE_END
