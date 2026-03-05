#pragma once

/// @file BitTwiddling.h

#include <functional>

#include <LanguageSupport.h>
#include <Numeric.h>

// NOLINTBEGIN(readability-magic-numbers)

namespace sys
{
    /// @brief The next power-of-two signed 32-bit integer.
    constexpr i32 nr2i32(i32 v) noexcept
    {
        v--;
        v |= v >> 1_i32;
        v |= v >> 2_i32;
        v |= v >> 4_i32;
        v |= v >> 8_i32;
        v |= v >> 16_i32;
        v++;
        return v;
    }
    /// @brief The next power-of-two signed 64-bit integer.
    constexpr i64 nr2i64(i64 v) noexcept
    {
        v--;
        v |= v >> 1_i64;
        v |= v >> 2_i64;
        v |= v >> 4_i64;
        v |= v >> 8_i64;
        v |= v >> 16_i64;
        v |= v >> 32_i64;
        v++;
        return v;
    }

    /// @brief Obtain bit-equal signed 16-bit integer from two bytes.
    constexpr i16 s16fb2(u8 msb, u8 lsb) noexcept { return i16((u16(msb, unsafe()) << 8_u16) | u16(lsb, unsafe()), unsafe()); }
    /// @brief Obtain the high byte from a signed 16-bit integer.
    constexpr u8 hbfs16(i16 val) noexcept { return u8(u16(val, unsafe()) >> 8_u16, unsafe()); }
    /// @brief Obtain the low byte from a signed 16-bit integer.
    constexpr u8 lbfs16(i16 val) noexcept { return u8(val, unsafe()); }

    /// @brief Hash combiner for `std::hash<T>(t)` and `std::hash<U>(u)`.
    template <typename T, typename U>
    constexpr sz dhc2(const T& t, const U& u) noexcept(noexcept(std::hash<T>()(t)) && noexcept(std::hash<U>()(u)))
    {
        sz seed = std::hash<T>()(t);
        return seed ^ (std::hash<U>()(u) + 0x9e3779b9_uz + (seed << 6_uz) + (seed >> 2_uz));
    }
} // namespace sys

// NOLINTEND(readability-magic-numbers)
