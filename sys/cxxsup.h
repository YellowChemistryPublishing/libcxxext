#pragma once

#include <limits>
#include <numeric>
#include <utility>

#include <LanguageSupport.h>
#include <Result.h>

#define _assert_ctor_can_fail() static_assert(false, "This constructor may fail, use `...::ctor` instead.")

namespace sys
{
    template <INumberUnderlying T, INumberUnderlying ValueType>
    constexpr T numeric_cast(ValueType value, unsafe)
    {
#if !defined(_MSC_VER) || !_MSC_VER
        return std::saturate_cast<T>(value);
#else
        if (std::cmp_less_equal(std::numeric_limits<T>::lowest(), value) && std::cmp_less_equal(value, std::numeric_limits<T>::max())) [[likely]]
            return T(value);
        else if (std::cmp_less(value, std::numeric_limits<T>::lowest()))
            return std::numeric_limits<T>::lowest();
        else // if (std::cmp_greater(value, std::numeric_limits<T>::max()))
            return std::numeric_limits<T>::max();
#endif
    }
    template <INumberUnderlying T, INumberUnderlying ValueType>
    constexpr result<T> numeric_cast(ValueType value)
    {
        if (std::cmp_less_equal(std::numeric_limits<T>::lowest(), value) && std::cmp_less_equal(value, std::numeric_limits<T>::max())) [[likely]]
            return T(value);
        else
            return nullptr;
    }

    constexpr i32 nr2i32(i32 v)
    {
        v--;
        v |= v >> 1u;
        v |= v >> 2u;
        v |= v >> 4u;
        v |= v >> 8u;
        v |= v >> 16u;
        v++;
        return v;
    }
    constexpr i64 nr2i64(i64 v)
    {
        v--;
        v |= v >> 1u;
        v |= v >> 2u;
        v |= v >> 4u;
        v |= v >> 8u;
        v |= v >> 16u;
        v |= v >> 32u;
        v++;
        return v;
    }

    /// @brief Obtain the two's complement signed 16-bit integer from two bytes.
    /// @param msb The most significant byte.
    /// @param lsb The least significant byte.
    /// @return Signed 16-bit integer.
    constexpr i16 s16fb2(u8 msb, u8 lsb)
    {
        return (i16(msb) << 8u) | lsb;
    }
    /// @brief Obtain the high byte from a signed 16-bit integer.
    /// @param val The signed 16-bit integer.
    /// @return The high byte.
    constexpr u8 hbfs16(i16 val)
    {
        return u8(+(val >> 8u));
    }
    /// @brief Obtain the low byte from a signed 16-bit integer.
    /// @param val The signed 16-bit integer.
    /// @return The low byte.
    constexpr u8 lbfs16(i16 val)
    {
        return u8(+val);
    }

    template <typename T, typename U>
    constexpr size_t dhc2(const T& t, const U& u)
    {
        size_t seed = std::hash<T>()(t);
        return seed ^ (std::hash<U>()(u) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }
} // namespace sys
