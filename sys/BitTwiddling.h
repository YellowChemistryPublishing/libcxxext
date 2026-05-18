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
    /// @brief The next power-of-two signed 32-bit integer.
    [[nodiscard]] constexpr i32 nr2i32(i32 v) noexcept
    {
        --v;
        v |= v >> 1_i32;
        v |= v >> 2_i32;
        v |= v >> 4_i32;
        v |= v >> 8_i32;
        v |= v >> 16_i32;
        ++v;
        return v;
    }
    /// @ingroup sys
    /// @brief The next power-of-two signed 64-bit integer.
    [[nodiscard]] constexpr i64 nr2i64(i64 v) noexcept
    {
        --v;
        v |= v >> 1_i64;
        v |= v >> 2_i64;
        v |= v >> 4_i64;
        v |= v >> 8_i64;
        v |= v >> 16_i64;
        v |= v >> 32_i64;
        ++v;
        return v;
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
