#pragma once

/// @file

#include <concepts>
#include <type_traits>

#include <meta/Type.h>
#include <traits/TriviallyRelocatable.h>

namespace sys
{
    /// @ingroup sys
    /// @brief Interface for object types.
    template <typename T>
    concept IObject = std::is_object_v<T>;

    /// @ingroup sys
    /// @brief Interface for functor types that can be nothrow invoked.
    template <typename Func, typename... Args>
    concept INothrowCallable = std::is_nothrow_invocable_v<Func, Args...> &&
        (std::same_as<std::remove_cv_t<std::invoke_result_t<Func, Args...>>, void> || std::is_nothrow_destructible_v<std::invoke_result_t<Func, Args...>>);

    template <typename T, typename... From>
    concept IConstructibleFrom = std::constructible_from<T, From...>;

    template <typename T, typename... From>
    concept INothrowConstructibleFrom = std::is_destructible_v<T> && std::is_nothrow_constructible_v<T, From...>;

    template <typename T>
    concept ITriviallyRelocatable = meta::type<T>::is_unqualified() && (std::is_trivial_v<T> || std::derived_from<T, traits::trivially_relocatable<T>>);
} // namespace sys
