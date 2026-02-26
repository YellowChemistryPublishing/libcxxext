#pragma once

/// @file Numeric.inl

#include <concepts>
#include <limits>
#include <utility>

#include <Result.h>
#include <Traits.h>

namespace sys
{
    /// @brief Exact-value cast `From` to `To`, or error if the value is out of range.
    template <std::integral To, std::integral From>
    constexpr result<To> bnumeric_cast(From value) noexcept // NOLINT(misc-use-internal-linkage)
    {
        if (std::cmp_less(value, std::numeric_limits<To>::lowest()) || std::cmp_greater(value, std::numeric_limits<To>::max())) [[unlikely]]
            return nullptr;
        else [[likely]]
            return To(value);
    }

    /// @brief Returns an opinionated sentinel value for `T`.
    template <sys::IBuiltinFloatingPoint T>
    consteval T bsentinel() noexcept // NOLINT(misc-use-internal-linkage)
    {
        return std::numeric_limits<T>::quiet_NaN();
    }
    /// @brief Returns an opinionated sentinel value for `T`.
    template <sys::ICharacter T>
    consteval T bsentinel() noexcept // NOLINT(misc-use-internal-linkage)
    {
        return 0;
    }

} // namespace sys
