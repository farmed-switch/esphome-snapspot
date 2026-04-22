

#pragma once

#include <cstddef>

#include <nlohmann/detail/abi_macros.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN
namespace detail
{

struct position_t
{

    std::size_t chars_read_total = 0;

    std::size_t chars_read_current_line = 0;

    std::size_t lines_read = 0;

    constexpr operator size_t() const
    {
        return chars_read_total;
    }
};

}
NLOHMANN_JSON_NAMESPACE_END
