#pragma once

/// @file

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <LanguageSupport.h>
#include <Platform.h>
#include <meta/Builtin.h>

/// @defgroup builtin_integers Built-in Integer Types
/// @brief Abbreviated built-in integer types.
/// @note Pass `byval`.
/// @{

// NOLINTBEGIN(google-runtime-int)
using byte = unsigned char;
using sbyte = signed char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;
using llong = long long;
using ullong = unsigned long long;
// NOLINTEND(google-runtime-int)

/// @}

namespace sys
{
    /// @brief Opinionated sentinel value for `T`.
    template <sys::IBuiltinIntegerSigned T>
    /* NOLINT(misc-use-internal-linkage) */ consteval T bsentinel() noexcept
    {
        return std::numeric_limits<T>::lowest();
    }
    /// @brief Opinionated sentinel value for `T`.
    template <sys::IBuiltinIntegerUnsigned T>
    /* NOLINT(misc-use-internal-linkage) */ consteval T bsentinel() noexcept
    {
        return std::numeric_limits<T>::max();
    }

    /// @brief Integer-to-integer cast.
    /// @warning `unsafe` because this is saturating!
    template <sys::IBuiltinInteger To, sys::IBuiltinInteger From>
    /* NOLINT(misc-use-internal-linkage) */ constexpr To bnumeric_cast(const From value, unsafe) noexcept
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
    /// @brief Floating-point-to-integer cast.
    /// @warning `unsafe` because this is saturating!
    template <sys::IBuiltinInteger To, sys::IBuiltinFloatingPoint From>
    /* NOLINT(misc-use-internal-linkage) */ constexpr To bnumeric_cast(const From value, unsafe) noexcept
    {
        if (!std::isfinite(value)) [[unlikely]]
            return sys::bsentinel<To>();
        else if (value <= _as(From, std::numeric_limits<To>::lowest())) [[unlikely]]
            return std::numeric_limits<To>::lowest();
        else if (value >= _as(From, std::numeric_limits<To>::max())) [[unlikely]]
            return std::numeric_limits<To>::max();
        else [[likely]]
            return To(value);
    }

    /// @brief Safe(r) high-level integer wrapper.
    /// @tparam For Built-in integer type.
    /// @details
    /// Provides saturating construction by default, with unsafe well-defined truncating variants.
    /// Negation, div-by-zero is also saturating.
    /// Mod-by-zero is identity.
    /// All arithmetic is wrapping otherwise.
    /// All shifts are logical, with shift-by-negative as opposite-direction shift.
    /// @note Pass `byval`.
    template <sys::IBuiltinInteger For>
    /* NOLINT(misc-use-internal-linkage) */ struct alignas(For) integer final
    {
    private:
        using signed_t = std::make_signed_t<For>;
        using unsigned_t = std::make_unsigned_t<For>;

        For underlying = 0;

        [[nodiscard]] constexpr unsigned_t u() const noexcept { return std::bit_cast<unsigned_t>(**this); }
    public:
        using underlying_type = For;

        [[nodiscard]] static consteval bool is_signed() noexcept { return std::is_signed_v<For>; }

        [[nodiscard]] static consteval integer highest() noexcept { return std::numeric_limits<For>::max(); }
        [[nodiscard]] static consteval integer lowest() noexcept { return std::numeric_limits<For>::lowest(); }
        /// @brief All ones.
        [[nodiscard]] static consteval integer ones() noexcept { return _as(For, ~_as(For, 0)); }
        /// @brief An opinionated sentinel value.
        [[nodiscard]] static consteval integer sentinel() noexcept { return sys::bsentinel<For>(); }

        constexpr integer() noexcept = default;
        /// @brief From narrower-bounded.
        template <IBuiltinIntegerCanHold<For> T>
        constexpr /* NOLINT(hicpp-explicit-conversions) */ integer(T v) noexcept : underlying(_as(For, v))
        { }
        /// @note Saturating.
        template <typename T>
        requires ((!IBuiltinIntegerCanHold<T, For> && sys::IBuiltinInteger<T>) || sys::IBuiltinFloatingPoint<T>)
        constexpr explicit integer(T v) noexcept : underlying(bnumeric_cast<For>(v, unsafe()))
        { }
        /// @warning `unsafe` because this is truncating.
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(T v, unsafe) noexcept : underlying(std::bit_cast<For>(_as(unsigned_t, std::bit_cast<std::make_unsigned_t<T>>(v))))
        { }
        /// @note Saturating.
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(integer<T> v) noexcept : integer(*v)
        { }
        /// @warning `unsafe` because this is truncating.
        template <sys::IBuiltinInteger T>
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
        [[nodiscard]] constexpr For& operator*() noexcept { return this->underlying; }

        /// @brief Saturating conversion to any built-in integer `T`.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return bnumeric_cast<T>(**this, unsafe());
        }
        /// @brief _Implicit_ conversion to underlying type.
        [[nodiscard]] constexpr /* NOLINT(hicpp-explicit-conversions) */ operator For() const noexcept { return **this; }
        /// @brief Conversion to any floating-point `T`.
        template <sys::IBuiltinFloatingPoint T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return _as(T, **this);
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return **this; }
        [[nodiscard]] constexpr bool operator!() const noexcept { return !**this; }

        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator==(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_equal(*a, *b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator==(integer<For> a, T b) noexcept
        {
            return std::cmp_equal(*a, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator==(For a, integer<T> b) noexcept
        {
            return std::cmp_equal(a, *b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_less(*a, *b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<(integer<For> a, T b) noexcept
        {
            return std::cmp_less(*a, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<(For a, integer<T> b) noexcept
        {
            return std::cmp_less(a, *b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<=(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_less_equal(*a, *b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<=(integer<For> a, T b) noexcept
        {
            return std::cmp_less_equal(*a, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<=(For a, integer<T> b) noexcept
        {
            return std::cmp_less_equal(a, *b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_greater(*a, *b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>(integer<For> a, T b) noexcept
        {
            return std::cmp_greater(*a, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>(For a, integer<T> b) noexcept
        {
            return std::cmp_greater(a, *b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>=(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_greater_equal(*a, *b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>=(integer<For> a, T b) noexcept
        {
            return std::cmp_greater_equal(*a, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>=(For a, integer<T> b) noexcept
        {
            return std::cmp_greater_equal(a, *b);
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
        /// @note Be warned that the negation of a signed `integer<...>::lowest()` is not UB but instead `integer<...>::highest()`.
        [[nodiscard]] constexpr integer operator-() const noexcept
        {
            if constexpr (std::is_unsigned_v<For>)
                []<bool Flag = false>() { static_assert(Flag, "Unsigned negation is intentionally disallowed."); }();

            if (**this == std::numeric_limits<For>::lowest()) [[unlikely]]
                return integer(std::numeric_limits<For>::max());
            else [[likely]]
                return integer(_as(For, -**this));
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
                return integer(_as(For, *a / *b));
        }
        [[nodiscard]] friend constexpr integer operator%(integer a, integer b) noexcept
        {
            if (*b == 0) [[unlikely]]
                return a;
            else [[likely]]
                return integer(_as(For, *a % *b));
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

    // Just to be sure.
    static_assert(sizeof(integer<uint_least8_t>) == sizeof(uint_least8_t));
    static_assert(alignof(integer<uint_least8_t>) == alignof(uint_least8_t));
} // namespace sys

/// @defgroup integers Safe(r) Integer Types
/// @brief Convenience aliases for `sys::integer<...>`.
/// @note Pass `byval`.
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

/// @defgroup integer_literals Integer Literals
/// @brief Literal suffixes for `sys::integer<...>`.
/// @note Pass `byval`.
/// @{

// NOLINTBEGIN(bugprone-exception-escape)
// clang-format off: C++23 -- no space b/w "" and literal suffix.
/// @brief Literal suffix for `i8`.
/* NOLINT(misc-use-internal-linkage) */ consteval i8 operator""_i8(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least8_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least8_t>::min()))
        throw std::overflow_error("Literal too large for `i8`.");

    return { _as(int_least8_t, lit) };
}
/// @brief Literal suffix for `i16`.
/* NOLINT(misc-use-internal-linkage) */ consteval i16 operator""_i16(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least16_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least16_t>::min()))
        throw std::overflow_error("Literal too large for `i16`.");

    return { _as(int_least16_t, lit) };
}
/// @brief Literal suffix for `i32`.
/* NOLINT(misc-use-internal-linkage) */ consteval i32 operator""_i32(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least32_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least32_t>::min()))
        throw std::overflow_error("Literal too large for `i32`.");

    return { _as(int_least32_t, lit) };
}
/// @brief Literal suffix for `i64`.
/* NOLINT(misc-use-internal-linkage) */ consteval i64 operator""_i64(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least64_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least64_t>::min()))
        throw std::overflow_error("Literal too large for `i64`.");

    return { _as(int_least64_t, lit) };
}
/// @brief Literal suffix for `u8`.
/* NOLINT(misc-use-internal-linkage) */ consteval u8 operator""_u8(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least8_t>::max()))
        throw std::overflow_error("Literal too large for `u8`.");

    return { _as(uint_least8_t, lit) };
}
/// @brief Literal suffix for `u16`.
/* NOLINT(misc-use-internal-linkage) */ consteval u16 operator""_u16(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least16_t>::max()))
        throw std::overflow_error("Literal too large for `u16`.");

    return { _as(uint_least16_t, lit) };
}
/// @brief Literal suffix for `u32`.
/* NOLINT(misc-use-internal-linkage) */ consteval u32 operator""_u32(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least32_t>::max()))
        throw std::overflow_error("Literal too large for `u32`.");

    return { _as(uint_least32_t, lit) };
}
/// @brief Literal suffix for `u64`.
/* NOLINT(misc-use-internal-linkage) */ consteval u64 operator""_u64(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least64_t>::max()))
        throw std::overflow_error("Literal too large for `u64`.");

    return { _as(uint_least64_t, lit) };
}
/// @brief Literal suffix for `ssz`.
/* NOLINT(misc-use-internal-linkage) */ consteval ssz operator""_z(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<ptrdiff_t>::max()) || std::cmp_less(lit, std::numeric_limits<ptrdiff_t>::min()))
        throw std::overflow_error("Literal too large for `ssz`.");

    return { _as(ptrdiff_t, lit) };
}
/// @brief Literal suffix for `sz`.
/* NOLINT(misc-use-internal-linkage) */ consteval sz operator""_uz(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<size_t>::max()))
        throw std::overflow_error("Literal too large for `sz`.");

    return { _as(size_t, lit) };
}
// clang-format on
// NOLINTEND(bugprone-exception-escape)

/// @}
