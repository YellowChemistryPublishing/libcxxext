#pragma once

/// @file

#include <functional>

#include <Integer.h>
#include <LanguageSupport.h>
#include <meta/Type.h>

// NOLINTBEGIN(readability-magic-numbers)

namespace sys
{
    /// @ingroup sys
    /// @brief The next power-of-two integer, or `0` if unrepresentable, or for `bit_ceil(0) == 0`.
    template <typename T>
    [[nodiscard]] constexpr integer<T> bit_ceil(const integer<T> v) noexcept
    {
        if (v <= T(0) || v >= (integer<T>(T(1)) << (integer<T>::template bits<integer<T>>() - integer<T>(T(1)))))
            return integer<T>(T(0));
        return integer<T>(std::bit_ceil(std::bit_cast<std::make_unsigned_t<T>>(*v)), unsafe);
    }

    /// @ingroup sys
    /// @brief Obtain bit-equal signed 16-bit integer from two bytes.
    [[nodiscard]] constexpr i16 s16fb2(u8 msb, u8 lsb) noexcept { return i16((u16(msb, unsafe) << 8_u16) | u16(lsb, unsafe), unsafe); }
    /// @ingroup sys
    /// @brief Obtain the high byte from a signed 16-bit integer.
    [[nodiscard]] constexpr u8 hbfs16(i16 val) noexcept { return u8(u16(val, unsafe) >> 8_u16, unsafe); }
    /// @ingroup sys
    /// @brief Obtain the low byte from a signed 16-bit integer.
    [[nodiscard]] constexpr u8 lbfs16(i16 val) noexcept { return u8(val, unsafe); }

    /// @ingroup sys
    /// @brief Hash combiner for `std::hash<decltype(a)>(a)` and `std::hash<decltype(b)>(b)`.
    [[nodiscard]] constexpr sz dhc2(auto&& a, auto&& b) noexcept(noexcept(std::hash<_decltype_of(a)>()(a)) && noexcept(std::hash<_decltype_of(b)>()(b)))
    {
        sz seed = std::hash<_decltype_of(a)>()(a);
        return seed ^ (std::hash<_decltype_of(b)>()(b) + 0x9e3779b9_uz + (seed << 6_uz) + (seed >> 2_uz));
    }
} // namespace sys

// NOLINTEND(readability-magic-numbers)
