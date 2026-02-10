#pragma once

#include <concepts>
#include <limits>
#include <utility>

#include <Result.h>
#include <Traits.h>

namespace sys
{
    /// @brief Exact-value cast `From` to `To`, or error if the value is out of range.
    template <std::integral To, std::integral From>
    constexpr result<To> bnumeric_cast(From value) noexcept
    {
        if (std::cmp_less(value, std::numeric_limits<To>::lowest()) || std::cmp_greater(value, std::numeric_limits<To>::max())) [[unlikely]]
            return nullptr;
        else [[likely]]
            return To(value);
    }

    template <sys::IBuiltinFloatingPoint T>
    consteval T bsentinel() noexcept
    {
        return std::numeric_limits<T>::quiet_NaN();
    }
    template <sys::ICharacter T>
    consteval T bsentinel() noexcept
    {
        return 0;
    }

} // namespace sys
