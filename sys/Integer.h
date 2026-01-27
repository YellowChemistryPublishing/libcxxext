#pragma once

#include <bit>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <LanguageSupport.h>
#include <Platform.h>
#include <Traits.h>

/// @defgroup BuiltinInteger Built-in Integer Types
/// @details Abbreviated built-in integer types.
/// @note Pass `byval`.

/// @addtogroup BuiltinInteger
/// @{

using byte = unsigned char;
using sbyte = signed char;
using ushort = unsigned short; // NOLINT(google-runtime-int)
using uint = unsigned int;
using ulong = unsigned long;       // NOLINT(google-runtime-int)
using llong = long long;           // NOLINT(google-runtime-int)
using ullong = unsigned long long; // NOLINT(google-runtime-int)

/// @}

namespace sys
{
    /// @brief Saturating integer-to-integer cast.
    template <std::integral To, std::integral From>
    constexpr To numeric_cast(const From value, unsafe)
    {
#if !_libcxxext_compiler_clang && defined(__cpp_lib_saturation_arithmetic) && __cpp_lib_saturation_arithmetic >= 202311l
        return std::saturate_cast<To>(value);
#else
        if (std::cmp_less(value, std::numeric_limits<To>::lowest())) [[unlikely]]
            return std::numeric_limits<To>::lowest();
        else if (std::cmp_greater(value, std::numeric_limits<To>::max())) [[unlikely]]
            return std::numeric_limits<To>::max();
        else [[likely]]
            return To(value);
#endif
    }
    /// @brief Saturating floating-point-to-integer cast.
    template <std::integral To, std::floating_point From>
    constexpr To numeric_cast(const From value, unsafe)
    {
        if (!std::isfinite(value) || value <= _as(From, std::numeric_limits<To>::lowest())) [[unlikely]]
            return std::numeric_limits<To>::lowest();
        else if (value >= _as(From, std::numeric_limits<To>::max())) [[unlikely]]
            return std::numeric_limits<To>::max();
        else [[likely]]
            return To(value);
    }

    /// @brief Safe(r) high-level integer wrapper.
    ///
    /// @tparam For Built-in integer type.
    /// @details
    /// Provides saturating construction by default, with unsafe well-defined truncating variants.
    /// Negation, div-by-zero is also saturating.
    /// Mod-by-zero is identity.
    /// All arithmetic is wrapping otherwise.
    /// All shifts are logical, with shift-by-negative as opposite-direction shift.
    /// @note Pass `byval`.
    template <std::integral For>
    struct _packed alignas(For) integer
    {
    private:
        using signed_t = std::make_signed_t<For>;
        using unsigned_t = std::make_unsigned_t<For>;

        For underlying = 0;

        [[nodiscard]] constexpr unsigned_t u() const noexcept { return std::bit_cast<unsigned_t>(**this); }
    public:
        using underlying_type = For;

        [[nodiscard]] _pure_const static consteval integer highest() { return integer(std::numeric_limits<For>::max()); }
        [[nodiscard]] _pure_const static consteval integer lowest() { return integer(std::numeric_limits<For>::lowest()); }
        [[nodiscard]] _pure_const static consteval bool is_signed() { return std::is_signed_v<For>; }

        constexpr integer() noexcept = default;
        template <IBuiltinIntegerCanHold<For> T>
        constexpr integer(T v) noexcept : underlying(_as(For, v)) // NOLINT(hicpp-explicit-conversions)
        { }
        template <typename T>
        requires std::integral<T> || std::floating_point<T>
        constexpr explicit integer(T v) noexcept : underlying(numeric_cast<For>(v, unsafe()))
        { }
        template <std::integral T>
        constexpr explicit integer(T v, unsafe) noexcept : underlying(v & ~_as(For, 0))
        { }
        template <std::integral T>
        constexpr explicit integer(integer<T> v) noexcept : integer(*v)
        { }
        template <std::integral T>
        constexpr explicit integer(integer<T> v, unsafe) noexcept : integer(*v, unsafe())
        { }
        constexpr integer(const integer& v) noexcept = default;
        constexpr integer(integer&& v) noexcept = default;
        constexpr ~integer() noexcept = default;

        template <IBuiltinIntegerCanHold<For> T>
        constexpr integer& operator=(T v) noexcept
        {
            this->underlying = _as(For, v);
            return *this;
        }
        constexpr integer& operator=(const integer& other) noexcept = default;
        constexpr integer& operator=(integer&& other) noexcept = default;

        [[nodiscard]] constexpr For operator*() const noexcept { return this->underlying; }
        template <std::integral T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return numeric_cast<T>(**this, unsafe());
        }
        [[nodiscard]] constexpr explicit operator For() const noexcept { return **this; }
        template <std::floating_point T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return _as(T, **this);
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return **this; }
        [[nodiscard]] constexpr bool operator!() const noexcept { return !**this; }

        template <std::integral T>
        [[nodiscard]] friend constexpr bool operator==(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_equal(*a, *b);
        }
        template <std::integral T>
        [[nodiscard]] friend constexpr bool operator<(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_less(*a, *b);
        }
        template <std::integral T>
        [[nodiscard]] friend constexpr bool operator<=(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_less_equal(*a, *b);
        }
        template <std::integral T>
        [[nodiscard]] friend constexpr bool operator>(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_greater(*a, *b);
        }
        template <std::integral T>
        [[nodiscard]] friend constexpr bool operator>=(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_greater_equal(*a, *b);
        }

        constexpr integer& operator++() noexcept { return (*this = integer(this->u() + _as(unsigned_t, 1), unsafe())); }
        constexpr integer& operator--() noexcept { return (*this = integer(this->u() - _as(unsigned_t, 1), unsafe())); }
        constexpr integer operator++(int) noexcept
        {
            integer ret = *this;
            *this = integer(this->u() + _as(unsigned_t, 1), unsafe());
            return ret;
        }
        constexpr integer operator--(int) noexcept
        {
            integer ret = *this;
            *this = integer(this->u() - _as(unsigned_t, 1), unsafe());
            return ret;
        }

        [[nodiscard]] constexpr integer operator+() const noexcept { return *this; }
        /// @note Be warned that the negation of `integer<...>::lowest()` is not UB but instead `integer<...>::highest()`.
        [[nodiscard]] constexpr integer operator-() const noexcept
        {
            if constexpr (std::is_unsigned_v<For>)
                []<bool Flag = false>() { static_assert(Flag, "Unsigned negation is intentionally disallowed."); }();

            if (**this == std::numeric_limits<For>::lowest()) [[unlikely]]
                return integer(std::numeric_limits<For>::max());
            else [[likely]]
                return integer(-**this);
        }
        [[nodiscard]] friend constexpr integer operator+(integer a, integer b) noexcept { return integer(a.u() + b.u(), unsafe()); }
        [[nodiscard]] friend constexpr integer operator-(integer a, integer b) noexcept { return integer(a.u() - b.u(), unsafe()); }
        [[nodiscard]] friend constexpr integer operator*(integer a, integer b) noexcept { return integer(a.u() * b.u(), unsafe()); }
        [[nodiscard]] friend constexpr integer operator/(integer a, integer b) noexcept
        {
            if (*b == 0) [[unlikely]]
            {
                if constexpr (std::is_signed_v<For>)
                    if (*a < 0)
                        return integer(std::numeric_limits<For>::lowest());

                return integer(std::numeric_limits<For>::max());
            }
            else [[likely]]
                return integer(*a / *b);
        }
        [[nodiscard]] friend constexpr integer operator%(integer a, integer b) noexcept
        {
            if (*b == 0) [[unlikely]]
                return a;
            else [[likely]]
                return integer(*a % *b);
        }

        [[nodiscard]] constexpr integer operator~() const noexcept { return integer(~this->u(), unsafe()); }
        [[nodiscard]] friend constexpr integer operator&(integer a, integer b) noexcept { return integer(a.u() & b.u(), unsafe()); }
        [[nodiscard]] friend constexpr integer operator|(integer a, integer b) noexcept { return integer(a.u() | b.u(), unsafe()); }
        [[nodiscard]] friend constexpr integer operator^(integer a, integer b) noexcept { return integer(a.u() ^ b.u(), unsafe()); }
        [[nodiscard]] friend constexpr integer operator<<(integer a, integer b) noexcept
        {
            if constexpr (std::is_signed_v<For>)
                if (*b < 0) [[unlikely]]
                    return integer(a.u() >> (-b).u(), unsafe()); // Note: `integer<...>::operator-()` produces signed max for negation of signed min, so safe.

            return integer(a.u() << b.u(), unsafe());
        }
        [[nodiscard]] friend constexpr integer operator>>(integer a, integer b) noexcept
        {
            if constexpr (std::is_signed_v<For>)
                if (*b < 0) [[unlikely]]
                    return integer(a.u() << (-b).u(), unsafe()); // Note: `integer<...>::operator-()` produces signed max for negation of signed min, so safe.

            return integer(a.u() >> b.u(), unsafe());
        }

        constexpr integer& operator+=(const integer& other) noexcept { return (*this = *this + other); }
        constexpr integer& operator-=(const integer& other) noexcept { return (*this = *this - other); }
        constexpr integer& operator*=(const integer& other) noexcept { return (*this = *this * other); }
        constexpr integer& operator/=(const integer& other) noexcept { return (*this = *this / other); }
        constexpr integer& operator%=(const integer& other) noexcept { return (*this = *this % other); }
        constexpr integer& operator&=(const integer& other) noexcept { return (*this = *this & other); }
        constexpr integer& operator|=(const integer& other) noexcept { return (*this = *this | other); }
        constexpr integer& operator^=(const integer& other) noexcept { return (*this = *this ^ other); }
        constexpr integer& operator<<=(const integer& other) noexcept { return (*this = *this << other); }
        constexpr integer& operator>>=(const integer& other) noexcept { return (*this = *this >> other); }
    };
} // namespace sys

/// @defgroup Integer Safe(r) Integer Types
/// @details Convenience aliases for `sys::integer<...>`.
/// @note Pass `byval`.

/// @addtogroup Integer
/// @{

using i8 = ::sys::integer<int_least8_t>;
using i16 = ::sys::integer<int_least16_t>;
using i32 = ::sys::integer<int_least32_t>;
using i64 = ::sys::integer<int_least64_t>;

using u8 = ::sys::integer<uint_least8_t>;
using u16 = ::sys::integer<uint_least16_t>;
using u32 = ::sys::integer<uint_least32_t>;
using u64 = ::sys::integer<uint_least64_t>;

using sz = ::sys::integer<size_t>;
using ssz = ::sys::integer<ptrdiff_t>;

/// @}

/// @defgroup IntegerLiterals Integer Literals
/// @details Literal suffixes for `sys::integer<...>`.
/// @note Pass `byval`.

/// @addtogroup IntegerLiterals
/// @{

// clang-format off: C++23 -- no space b/w "" and literal suffix.
consteval i8 operator""_i8(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least8_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least8_t>::min())) [[unlikely]]
        throw std::overflow_error("Literal too large for `i8`.");

    return { _as(int_least8_t, lit) };
}
consteval i16 operator""_i16(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least16_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least16_t>::min())) [[unlikely]]
        throw std::overflow_error("Literal too large for `i16`.");

    return { _as(int_least16_t, lit) };
}
consteval i32 operator""_i32(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least32_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least32_t>::min())) [[unlikely]]
        throw std::overflow_error("Literal too large for `i32`.");

    return { _as(int_least32_t, lit) };
}
consteval i64 operator""_i64(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least64_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least64_t>::min())) [[unlikely]]
        throw std::overflow_error("Literal too large for `i64`.");

    return { _as(int_least64_t, lit) };
}
consteval u8 operator""_u8(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least8_t>::max())) [[unlikely]]
        throw std::overflow_error("Literal too large for `u8`.");

    return { _as(uint_least8_t, lit) };
}
consteval u16 operator""_u16(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least16_t>::max())) [[unlikely]]
        throw std::overflow_error("Literal too large for `u16`.");

    return { _as(uint_least16_t, lit) };
}
consteval u32 operator""_u32(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least32_t>::max())) [[unlikely]]
        throw std::overflow_error("Literal too large for `u32`.");

    return { _as(uint_least32_t, lit) };
}
consteval u64 operator""_u64(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least64_t>::max())) [[unlikely]]
        throw std::overflow_error("Literal too large for `u64`.");

    return { _as(uint_least64_t, lit) };
}
consteval ssz operator""_z(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<ptrdiff_t>::max()) || std::cmp_less(lit, std::numeric_limits<ptrdiff_t>::min())) [[unlikely]]
        throw std::overflow_error("Literal too large for `ssz`.");

    return { _as(ptrdiff_t, lit) };
}
consteval sz operator""_uz(ullong lit)
{
    if (std::cmp_greater(lit, std::numeric_limits<size_t>::max())) [[unlikely]]
        throw std::overflow_error("Literal too large for `sz`.");

    return { _as(size_t, lit) };
}
// clang-format on

/// @}
