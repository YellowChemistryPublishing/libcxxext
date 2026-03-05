#pragma once

/// @file

#include <concepts>
#include <limits>
#include <utility>

#include <Result.h>
#include <meta/Builtin.h>

namespace sys
{
    /// @brief Exact-value cast `From` to `To`, or error if the value is out of range.
    template <std::integral To, std::integral From>
    /* NOLINT(misc-use-internal-linkage) */ constexpr result<To> bnumeric_cast(From value) noexcept
    {
        if (std::cmp_less(value, std::numeric_limits<To>::lowest()) || std::cmp_greater(value, std::numeric_limits<To>::max())) [[unlikely]]
            return nullptr;
        else [[likely]]
            return To(value);
    }

    /// @brief Returns an opinionated sentinel value for `T`.
    template <sys::IBuiltinFloatingPoint T>
    /* NOLINT(misc-use-internal-linkage) */ consteval T bsentinel() noexcept
    {
        return std::numeric_limits<T>::quiet_NaN();
    }
    /// @brief Returns an opinionated sentinel value for `T`.
    template <sys::ICharacter T>
    /* NOLINT(misc-use-internal-linkage) */ consteval T bsentinel() noexcept
    {
        return 0;
    }

} // namespace sys
