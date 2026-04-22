

#pragma once

#include <utility>

#include <nlohmann/detail/abi_macros.hpp>
#include <nlohmann/detail/conversions/from_json.hpp>
#include <nlohmann/detail/conversions/to_json.hpp>
#include <nlohmann/detail/meta/identity_tag.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN

template<typename ValueType, typename>
struct adl_serializer
{

    template<typename BasicJsonType, typename TargetType = ValueType>
    static auto from_json(BasicJsonType && j, TargetType& val) noexcept(
        noexcept(::nlohmann::from_json(std::forward<BasicJsonType>(j), val)))
    -> decltype(::nlohmann::from_json(std::forward<BasicJsonType>(j), val), void())
    {
        ::nlohmann::from_json(std::forward<BasicJsonType>(j), val);
    }

    template<typename BasicJsonType, typename TargetType = ValueType>
    static auto from_json(BasicJsonType && j) noexcept(
    noexcept(::nlohmann::from_json(std::forward<BasicJsonType>(j), detail::identity_tag<TargetType> {})))
    -> decltype(::nlohmann::from_json(std::forward<BasicJsonType>(j), detail::identity_tag<TargetType> {}))
    {
        return ::nlohmann::from_json(std::forward<BasicJsonType>(j), detail::identity_tag<TargetType> {});
    }

    template<typename BasicJsonType, typename TargetType = ValueType>
    static auto to_json(BasicJsonType& j, TargetType && val) noexcept(
        noexcept(::nlohmann::to_json(j, std::forward<TargetType>(val))))
    -> decltype(::nlohmann::to_json(j, std::forward<TargetType>(val)), void())
    {
        ::nlohmann::to_json(j, std::forward<TargetType>(val));
    }
};

NLOHMANN_JSON_NAMESPACE_END
