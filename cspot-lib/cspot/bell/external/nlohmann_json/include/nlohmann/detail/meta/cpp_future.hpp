

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <nlohmann/detail/macro_scope.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN
namespace detail
{

template<typename T>
using uncvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

#ifdef JSON_HAS_CPP_14

using std::enable_if_t;
using std::index_sequence;
using std::make_index_sequence;
using std::index_sequence_for;

#else

template<bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T, T... Ints>
struct integer_sequence
{
    using value_type = T;
    static constexpr std::size_t size() noexcept
    {
        return sizeof...(Ints);
    }
};

template <size_t... Ints>
using index_sequence = integer_sequence<size_t, Ints...>;

namespace utility_internal
{

template <typename Seq, size_t SeqSize, size_t Rem>
struct Extend;

template <typename T, T... Ints, size_t SeqSize>
struct Extend<integer_sequence<T, Ints...>, SeqSize, 0>
{
    using type = integer_sequence < T, Ints..., (Ints + SeqSize)... >;
};

template <typename T, T... Ints, size_t SeqSize>
struct Extend<integer_sequence<T, Ints...>, SeqSize, 1>
{
    using type = integer_sequence < T, Ints..., (Ints + SeqSize)..., 2 * SeqSize >;
};

template <typename T, size_t N>
struct Gen
{
    using type =
        typename Extend < typename Gen < T, N / 2 >::type, N / 2, N % 2 >::type;
};

template <typename T>
struct Gen<T, 0>
{
    using type = integer_sequence<T>;
};

}

template <typename T, T N>
using make_integer_sequence = typename utility_internal::Gen<T, N>::type;

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

template <typename... Ts>
using index_sequence_for = make_index_sequence<sizeof...(Ts)>;

#endif

template<unsigned N> struct priority_tag : priority_tag < N - 1 > {};
template<> struct priority_tag<0> {};

template<typename T>
struct static_const
{
    static JSON_INLINE_VARIABLE constexpr T value{};
};

#ifndef JSON_HAS_CPP_17
    template<typename T>
    constexpr T static_const<T>::value;
#endif

template<typename T, typename... Args>
inline constexpr std::array<T, sizeof...(Args)> make_array(Args&& ... args)
{
    return std::array<T, sizeof...(Args)> {{static_cast<T>(std::forward<Args>(args))...}};
}

}
NLOHMANN_JSON_NAMESPACE_END
