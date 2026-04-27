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
#include <Result.h>
#include <meta/Builtin.h>

namespace sys
{
    /// @ingroup sys
    /// @brief Opinionated sentinel value for `T`.
    template <sys::IBuiltinIntegerSigned T>
    requires (!sys::ICharacter<T>)
    /* NOLINT(misc-use-internal-linkage) */ consteval T bsentinel() noexcept
    {
        return std::numeric_limits<T>::lowest();
    }
    /// @ingroup sys
    /// @brief Opinionated sentinel value for `T`.
    template <sys::IBuiltinIntegerUnsigned T>
    requires (!sys::ICharacter<T>)
    /* NOLINT(misc-use-internal-linkage) */ consteval T bsentinel() noexcept
    {
        return std::numeric_limits<T>::max();
    }
    /// @ingroup sys
    /// @brief Returns an opinionated sentinel value for `T`.
    template <sys::IBuiltinFloatingPoint T>
    /* NOLINT(misc-use-internal-linkage) */ consteval T bsentinel() noexcept
    {
        return std::numeric_limits<T>::quiet_NaN();
    }
    /// @ingroup sys
    /// @brief Returns an opinionated sentinel value for `T`.
    template <sys::ICharacter T>
    /* NOLINT(misc-use-internal-linkage) */ consteval T bsentinel() noexcept
    {
        return 0;
    }

    /// @ingroup sys
    /// @brief Integer-to-integer cast.
    /// @warning `unsafe` because this is saturating!
    template <sys::IBuiltinInteger To, sys::IBuiltinInteger From>
    /* NOLINT(misc-use-internal-linkage) */ constexpr To bnumeric_cast(const From value, decltype(unsafe)) noexcept
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
    /// @ingroup sys
    /// @brief Floating-point-to-integer cast.
    /// @warning `unsafe` because this is saturating!
    template <sys::IBuiltinInteger To, sys::IBuiltinFloatingPoint From>
    /* NOLINT(misc-use-internal-linkage) */ constexpr To bnumeric_cast(const From value, decltype(unsafe)) noexcept
    {
        if (!std::isfinite(value)) [[unlikely]]
            return sys::bsentinel<To>();
        else if (value <= _as(std::numeric_limits<To>::lowest(), From)) [[unlikely]]
            return std::numeric_limits<To>::lowest();
        else if (value >= _as(std::numeric_limits<To>::max(), From)) [[unlikely]]
            return std::numeric_limits<To>::max();
        else [[likely]]
            return To(value);
    }

    /// @ingroup sys
    /// @brief Exact-value cast `From` to `To`, or error if the value is out of range.
    template <sys::IBuiltinInteger To, sys::IBuiltinInteger From>
    /* NOLINT(misc-use-internal-linkage) */ constexpr result<To> bnumeric_cast(From value) noexcept
    {
        if (std::cmp_less(value, std::numeric_limits<To>::lowest()) || std::cmp_greater(value, std::numeric_limits<To>::max())) [[unlikely]]
            return nullptr;
        else [[likely]]
            return To(value);
    }
    /// @ingroup sys
    /// @brief Exact-value cast `From` to `To`, or error if the value is out of range.
    template <sys::IBuiltinInteger To, sys::IBuiltinFloatingPoint From>
    /* NOLINT(misc-use-internal-linkage) */ constexpr result<To> bnumeric_cast(From value) noexcept
    {
        // Don't invert condition, need to catch NaN.
        if (value >= _as(std::numeric_limits<To>::lowest(), From) && value <= _as(std::numeric_limits<To>::max(), From)) [[likely]]
            return To(value);
        else [[unlikely]]
            return nullptr;
    }

    /// @ingroup sys
    /// @brief Safe(r) high-level integer wrapper.
    /// @tparam For Built-in integer type.
    /// @details
    /// Provides saturating construction by default, with unsafe well-defined truncating variants.
    /// Negation, div-by-zero is also saturating.
    /// Mod-by-zero is identity.
    /// All arithmetic is wrapping otherwise.
    /// All shifts are logical, with shift-by-negative as opposite-direction shift.
    /// Implements `sys::INothrowDefaultConstructible`, `sys::INothrowCopyConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowCopyAssignable`,
    /// `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`, `sys::IBooleanTestable`, `sys::INothrowEqualityComparable`, `sys::INothrowSwappable`.
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

        [[nodiscard]] static consteval bool is_signed() noexcept { return IBuiltinIntegerSigned<For>; }

        [[nodiscard]] static consteval integer highest() noexcept { return std::numeric_limits<For>::max(); }
        [[nodiscard]] static consteval integer lowest() noexcept { return std::numeric_limits<For>::lowest(); }
        /// @brief All one bits.
        [[nodiscard]] static consteval integer ones() noexcept { return _as(~_as(0, For), For); }
        /// @brief An opinionated sentinel value.
        [[nodiscard]] static consteval integer sentinel() noexcept { return sys::bsentinel<For>(); }

        constexpr integer() noexcept = default;
        /// @brief From narrower-bounded.
        template <IBuiltinIntegerCanHold<For> T>
        constexpr /* NOLINT(hicpp-explicit-conversions) */ integer(T v) noexcept : underlying(_as(v, For))
        { }
        /// @note Saturating.
        template <typename T>
        requires ((!IBuiltinIntegerCanHold<T, For> && sys::IBuiltinInteger<T>) || sys::IBuiltinFloatingPoint<T>)
        constexpr explicit integer(T v) noexcept : underlying(bnumeric_cast<For>(v, unsafe))
        { }
        /// @warning `unsafe` because this is truncating.
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(T v, decltype(unsafe)) noexcept : underlying(std::bit_cast<For>(_as(std::bit_cast<std::make_unsigned_t<T>>(v), unsigned_t)))
        { }
        /// @note Saturating.
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(integer<T> v) noexcept : integer(*v)
        { }
        /// @warning `unsafe` because this is truncating.
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(integer<T> v, decltype(unsafe)) noexcept : integer(*v, unsafe)
        { }
        constexpr integer(const integer& other) noexcept = default;
        constexpr integer(integer&& other) noexcept : integer(other) /* NOLINT(performance-move-constructor-init) */ { }
        constexpr ~integer() noexcept = default;

        template <typename T>
        requires IBuiltinIntegerCanHold<For, T>
        constexpr integer& operator=(T v) noexcept
        {
            this->underlying = _as(v, For);
            return *this;
        }
        constexpr integer& operator=(const integer& other) noexcept = default;
        constexpr integer& operator=(integer&& other) noexcept
        {
            *this = other;
            return *this;
        }

        [[nodiscard]] constexpr For operator*() const noexcept { return this->underlying; }
        [[nodiscard]] constexpr For& operator*() noexcept { return this->underlying; }

        /// @brief Saturating conversion to any built-in integer `T`.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return bnumeric_cast<T>(**this, unsafe);
        }
        /// @brief _Implicit_ conversion to underlying type.
        [[nodiscard]] constexpr /* NOLINT(hicpp-explicit-conversions) */ operator For() const noexcept { return **this; }
        /// @brief Conversion to any floating-point `T`.
        template <sys::IBuiltinFloatingPoint T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return _as(**this, T);
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

        constexpr integer& operator++() noexcept { return (*this = integer(this->u() + _as(1, unsigned_t), unsafe)); }
        constexpr integer& operator--() noexcept { return (*this = integer(this->u() - _as(1, unsigned_t), unsafe)); }
        constexpr integer operator++(int) noexcept
        {
            const integer ret = *this;
            *this = integer(this->u() + _as(1, unsigned_t), unsafe);
            return ret;
        }
        constexpr integer operator--(int) noexcept
        {
            const integer ret = *this;
            *this = integer(this->u() - _as(1, unsigned_t), unsafe);
            return ret;
        }

        [[nodiscard]] constexpr integer operator+() const noexcept { return *this; }
        /// @note Be warned that the negation of a signed `integer<...>::lowest()` is not UB but instead `integer<...>::highest()`.
        [[nodiscard]] constexpr integer operator-() const noexcept
        requires (IBuiltinIntegerSigned<For>) // Unsigned negation is intentionally disallowed.
        {
            if (**this == std::numeric_limits<For>::lowest()) [[unlikely]]
                return integer(std::numeric_limits<For>::max());
            else [[likely]]
                return integer(_as(-**this, For));
        }
        [[nodiscard]] friend constexpr integer operator+(integer a, integer b) noexcept { return integer(a.u() + b.u(), unsafe); }
        [[nodiscard]] friend constexpr integer operator-(integer a, integer b) noexcept { return integer(a.u() - b.u(), unsafe); }
        [[nodiscard]] friend constexpr integer operator*(integer a, integer b) noexcept { return integer(a.u() * b.u(), unsafe); }
        [[nodiscard]] friend constexpr integer operator/(integer a, integer b) noexcept
        {
            if (*b == 0) [[unlikely]]
            {
                if constexpr (IBuiltinIntegerSigned<For>)
                    if (*a < 0)
                        return integer(std::numeric_limits<For>::lowest());

                return integer(std::numeric_limits<For>::max());
            }
            else [[likely]]
                return integer(_as(*a / *b, For));
        }
        [[nodiscard]] friend constexpr integer operator%(integer a, integer b) noexcept
        {
            if (*b == 0) [[unlikely]]
                return a;
            else [[likely]]
                return integer(_as(*a % *b, For));
        }

        [[nodiscard]] constexpr integer operator~() const noexcept { return integer(~this->u(), unsafe); }
        [[nodiscard]] friend constexpr integer operator&(integer a, integer b) noexcept { return integer(a.u() & b.u(), unsafe); }
        [[nodiscard]] friend constexpr integer operator|(integer a, integer b) noexcept { return integer(a.u() | b.u(), unsafe); }
        [[nodiscard]] friend constexpr integer operator^(integer a, integer b) noexcept { return integer(a.u() ^ b.u(), unsafe); }
        [[nodiscard]] friend constexpr integer operator<<(integer a, integer b) noexcept
        {
            if constexpr (IBuiltinIntegerSigned<For>)
                if (*b < 0) [[unlikely]]
                    return integer(a.u() >> (-b).u(), unsafe); // Note: `integer<...>::operator-()` produces signed max for negation of signed min, so safe.

            return integer(a.u() << b.u(), unsafe);
        }
        [[nodiscard]] friend constexpr integer operator>>(integer a, integer b) noexcept
        {
            if constexpr (IBuiltinIntegerSigned<For>)
                if (*b < 0) [[unlikely]]
                    return integer(a.u() << (-b).u(), unsafe); // Note: `integer<...>::operator-()` produces signed max for negation of signed min, so safe.

            return integer(a.u() >> b.u(), unsafe);
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

/// @defgroup integers Safe(r) Integer Types
/// @ingroup tags
/// @ingroup sys
/// @brief Convenience aliases for `sys::integer<...>`.
/// @note Pass `byval`.
/// @{

/// @brief Fixed-width, signed integer of at least 8 bits.
using i8 = ::sys::integer<int_least8_t>;
/// @brief Fixed-width, signed integer of at least 16 bits.
using i16 = ::sys::integer<int_least16_t>;
/// @brief Fixed-width, signed integer of at least 32 bits.
using i32 = ::sys::integer<int_least32_t>;
/// @brief Fixed-width, signed integer of at least 64 bits.
using i64 = ::sys::integer<int_least64_t>;

/// @brief Fixed-width, unsigned integer of at least 8 bits.
using u8 = ::sys::integer<uint_least8_t>;
/// @brief Fixed-width, unsigned integer of at least 16 bits.
using u16 = ::sys::integer<uint_least16_t>;
/// @brief Fixed-width, unsigned integer of at least 32 bits.
using u32 = ::sys::integer<uint_least32_t>;
/// @brief Fixed-width, unsigned integer of at least 64 bits.
using u64 = ::sys::integer<uint_least64_t>;

/// @brief Unsigned size type.
using sz = ::sys::integer<size_t>;
/// @brief Signed size type.
using ssz = ::sys::integer<ptrdiff_t>;

/// @}

/// @defgroup integer_literals Integer Literals
/// @ingroup tags
/// @ingroup sys
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

    return { _as(lit, int_least8_t) };
}
/// @brief Literal suffix for `i16`.
/* NOLINT(misc-use-internal-linkage) */ consteval i16 operator""_i16(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least16_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least16_t>::min()))
        throw std::overflow_error("Literal too large for `i16`.");

    return { _as(lit, int_least16_t) };
}
/// @brief Literal suffix for `i32`.
/* NOLINT(misc-use-internal-linkage) */ consteval i32 operator""_i32(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least32_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least32_t>::min()))
        throw std::overflow_error("Literal too large for `i32`.");

    return { _as(lit, int_least32_t) };
}
/// @brief Literal suffix for `i64`.
/* NOLINT(misc-use-internal-linkage) */ consteval i64 operator""_i64(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least64_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least64_t>::min()))
        throw std::overflow_error("Literal too large for `i64`.");

    return { _as(lit, int_least64_t) };
}
/// @brief Literal suffix for `u8`.
/* NOLINT(misc-use-internal-linkage) */ consteval u8 operator""_u8(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least8_t>::max()))
        throw std::overflow_error("Literal too large for `u8`.");

    return { _as(lit, uint_least8_t) };
}
/// @brief Literal suffix for `u16`.
/* NOLINT(misc-use-internal-linkage) */ consteval u16 operator""_u16(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least16_t>::max()))
        throw std::overflow_error("Literal too large for `u16`.");

    return { _as(lit, uint_least16_t) };
}
/// @brief Literal suffix for `u32`.
/* NOLINT(misc-use-internal-linkage) */ consteval u32 operator""_u32(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least32_t>::max()))
        throw std::overflow_error("Literal too large for `u32`.");

    return { _as(lit, uint_least32_t) };
}
/// @brief Literal suffix for `u64`.
/* NOLINT(misc-use-internal-linkage) */ consteval u64 operator""_u64(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least64_t>::max()))
        throw std::overflow_error("Literal too large for `u64`.");

    return { _as(lit, uint_least64_t) };
}
/// @brief Literal suffix for `ssz`.
/* NOLINT(misc-use-internal-linkage) */ consteval ssz operator""_z(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<ptrdiff_t>::max()) || std::cmp_less(lit, std::numeric_limits<ptrdiff_t>::min()))
        throw std::overflow_error("Literal too large for `ssz`.");

    return { _as(lit, ptrdiff_t) };
}
/// @brief Literal suffix for `sz`.
/* NOLINT(misc-use-internal-linkage) */ consteval sz operator""_uz(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<size_t>::max()))
        throw std::overflow_error("Literal too large for `sz`.");

    return { _as(lit, size_t) };
}
// clang-format on
// NOLINTEND(bugprone-exception-escape)

/// @}
