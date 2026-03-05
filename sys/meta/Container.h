#pragma once

/// @file

#include <concepts>
#include <cstddef>
#include <iterator>
#include <type_traits>

#include <LanguageSupport.h>

namespace sys::meta
{
    /// @brief Check whether an empty-queryable `range` is empty.
    template <typename T>
    constexpr bool is_empty(const T& range)
    {
        if constexpr (requires { range.empty(); })
            return range.empty();
        else if constexpr (requires { std::size(range); })
            return std::size(range) == _as(decltype(std::size(range)), 0);
        else if constexpr (requires { requires false; })
        { }
    }
    /// @brief Inplace construct and append to an appendable `range`.
    template <typename T, typename... Args>
    constexpr decltype(auto) append_to(T& range, Args&&... args)
    {
        if constexpr (requires { range.emplace_back(std::forward<Args>(args)...); })
            return range.emplace_back(std::forward<Args>(args)...);
        else if constexpr (requires { range.push_back(std::forward<Args>(args)...); })
            return range.push_back(std::forward<Args>(args)...);
        else if constexpr (requires { range.push(std::forward<Args>(args)...); })
            return range.push(std::forward<Args>(args)...);
        else if constexpr (requires { range.append(std::forward<Args>(args)...); })
            return range.append(std::forward<Args>(args)...);
        else if constexpr (requires { (range << ... << std::forward<Args>(args)); })
            return (range << ... << std::forward<Args>(args));
        else if constexpr (requires { requires false; })
        { }
    }
} // namespace sys::meta

namespace sys
{
    /// @brief Whether `T` is sizeable.
    template <typename T, typename U = size_t>
    concept ISizeable = requires(T range) {
        requires !std::same_as<U, void>;
        { std::size(range) } -> std::same_as<U>;
    } || requires(T range) {
        requires std::same_as<U, void>;
        std::size(range);
    };

    /// @brief Whether `T` is iterable.
    template <typename T, typename U = void>
    concept IEnumerable = requires(T& range, std::remove_cvref_t<decltype(std::begin(range))> it) {
        std::begin(range);
        std::end(range);

        std::begin(range) != std::end(range);
        ++it;

        requires (requires {
            requires std::same_as<U, void>;
            *std::begin(range);
        } || std::same_as<std::remove_cvref_t<decltype(*std::begin(range))>, U>);
    };

    /// @brief Whether `T` can be checked for emptiness.
    template <typename T>
    concept IEmptyQueryable = requires(T range) { meta::is_empty(range); };
    /// @brief Whether `T` can be appended to.
    template <typename T, typename... U>
    concept IAppendable = requires(T range) {
        requires (sizeof...(U) > 0);
        meta::append_to(range, std::declval<U>()...);
    };
} // namespace sys
