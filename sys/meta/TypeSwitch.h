#pragma once

/// @file

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include <meta/Properties.h>

namespace sys::meta
{
    /// @brief Metadata type for `sys::meta::type_switch_cases<...>`.
    template <bool Condition, typename T>
    struct type_case final : meta_type
    {
        using type = T;

        /// @brief Whether `Condition` is `true`.
        static consteval bool is_early_return() { return Condition; }
    };
    /// @brief Metadata type for a type-`switch`.
    template <typename... Cases>
    struct type_switch_cases final : meta_type
    {
        using cases = std::tuple<Cases...>;
        using return_cases = decltype(std::tuple_cat(std::declval<std::conditional_t<Cases::is_early_return(), std::tuple<Cases>, std::tuple<>>>()...));

        template <size_t Index>
        using at = std::tuple_element_t<Index, cases>;

        /// @brief How many cases meet their conditions.
        static consteval size_t count_returns() { return (Cases::is_early_return() + ...); }
    };
    /// @brief The type of the first type-case that meets its condition.
    template <typename... Cases>
    using type_switch = std::tuple_element_t<0, typename type_switch_cases<Cases...>::return_cases>::type;
} // namespace sys::meta
