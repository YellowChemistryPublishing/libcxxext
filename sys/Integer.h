#pragma once

/// @file

#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <LanguageSupport.h>
#include <Numeric.h>
#include <meta/Builtin.h>

namespace sys
{
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
    /* NOLINT(misc-use-internal-linkage) */ struct integer final
    {
    private:
        using signed_t = std::make_signed_t<For>;
        using unsigned_t = std::make_unsigned_t<For>;

        For underlying = 0;

        [[nodiscard]] constexpr unsigned_t u() const noexcept { return _as(this->underlying, unsigned_t); }
    public:
        using underlying_type = For;

        [[nodiscard]] static consteval bool is_signed() noexcept { return IBuiltinIntegerSigned<For>; }
        template <typename T = integer<size_t>>
        [[nodiscard]] static constexpr T fixed_width() noexcept(noexcept(T(sizeof(For) * CHAR_BIT)))
        {
            return T(sizeof(For) * CHAR_BIT);
        }

        [[nodiscard]] static consteval integer highest() noexcept { return std::numeric_limits<For>::max(); }
        [[nodiscard]] static consteval integer lowest() noexcept { return std::numeric_limits<For>::lowest(); }
        /// @brief All one bits.
        [[nodiscard]] static consteval integer ones() noexcept { return _as(~_as(0, For), For); }
        /// @brief An opinionated sentinel value.
        [[nodiscard]] static consteval integer sentinel() noexcept { return sys::bsentinel<For>(); }

        constexpr integer() noexcept = default;
        /// @brief From narrower-bounded.
        template <IBuiltinIntegerNarrowerThan<For> T>
        constexpr /* NOLINT(hicpp-explicit-conversions) */ integer(T v) noexcept : underlying(_as(v, For))
        { }
        /// @note Saturating.
        template <typename T>
        requires ((!IBuiltinIntegerNarrowerThan<T, For> && sys::IBuiltinInteger<T>) || sys::IBuiltinFloatingPoint<T>)
        constexpr explicit integer(T v) noexcept : underlying(bnumeric_cast<For>(v, unsafe))
        { }
        /// @warning `unsafe` because this is truncating.
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(T v, decltype(unsafe)) noexcept : underlying(_as(v, For))
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
        constexpr integer(integer&&) noexcept = default;
        constexpr ~integer() noexcept = default;

        template <IBuiltinIntegerNarrowerThan<For> T>
        constexpr integer& operator=(T v) noexcept
        {
            this->underlying = _as(v, For);
            return *this;
        }
        constexpr integer& operator=(const integer& other) noexcept = default;
        constexpr integer& operator=(integer&&) noexcept = default;

        [[nodiscard]] constexpr For operator*() const noexcept { return this->underlying; }
        [[nodiscard]] constexpr For& operator*() noexcept { return this->underlying; }

        /// @brief Saturating conversion to any built-in integer `T`.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return bnumeric_cast<T>(this->underlying, unsafe);
        }
        /// @brief _Implicit_ conversion to underlying type.
        [[nodiscard]] constexpr /* NOLINT(hicpp-explicit-conversions) */ operator For() const noexcept { return this->underlying; }
        /// @brief Conversion to any floating-point `T`.
        template <sys::IBuiltinFloatingPoint T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return _as(this->underlying, T);
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return this->underlying; }
        [[nodiscard]] constexpr bool operator!() const noexcept { return !this->underlying; }

        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator==(integer<For> a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying == b.underlying;
            else
                return std::cmp_equal(a.underlying, b.underlying);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator==(integer<For> a, T b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying == b;
            else
                return std::cmp_equal(a.underlying, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator==(For a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a == b.underlying;
            else
                return std::cmp_equal(a, b.underlying);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<(integer<For> a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying < b.underlying;
            else
                return std::cmp_less(a.underlying, b.underlying);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<(integer<For> a, T b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying < b;
            else
                return std::cmp_less(a.underlying, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<(For a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a < b.underlying;
            else
                return std::cmp_less(a, b.underlying);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<=(integer<For> a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying <= b.underlying;
            else
                return std::cmp_less_equal(a.underlying, b.underlying);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<=(integer<For> a, T b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying <= b;
            else
                return std::cmp_less_equal(a.underlying, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<=(For a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a <= b.underlying;
            else
                return std::cmp_less_equal(a, b.underlying);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>(integer<For> a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying > b.underlying;
            else
                return std::cmp_greater(a.underlying, b.underlying);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>(integer<For> a, T b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying > b;
            else
                return std::cmp_greater(a.underlying, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>(For a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a > b.underlying;
            else
                return std::cmp_greater(a, b.underlying);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>=(integer<For> a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying >= b.underlying;
            else
                return std::cmp_greater_equal(a.underlying, b.underlying);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>=(integer<For> a, T b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a.underlying >= b;
            else
                return std::cmp_greater_equal(a.underlying, b);
        }
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>=(For a, integer<T> b) noexcept
        {
            if constexpr (std::same_as<T, For>)
                return a >= b.underlying;
            else
                return std::cmp_greater_equal(a, b.underlying);
        }

        constexpr integer& operator++() noexcept
        {
            this->underlying = _as(this->u() + _as(1, unsigned_t), For);
            return *this;
        }
        constexpr integer& operator--() noexcept
        {
            this->underlying = _as(this->u() - _as(1, unsigned_t), For);
            return *this;
        }
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
        /// @note Be warned that the negation of a signed `sys::integer<...>::lowest()` is not UB but instead `sys::integer<...>::highest()`.
        [[nodiscard]] constexpr integer operator-() const noexcept
        requires IBuiltinIntegerSigned<For> // Unsigned negation is intentionally disallowed.
        {
            return integer(this->underlying == std::numeric_limits<For>::lowest() ? std::numeric_limits<For>::max() : -this->underlying, unsafe);
        }
        [[nodiscard]] friend constexpr integer operator+(integer a, integer b) noexcept { return integer(_as(a.u() + b.u(), For), unsafe); }
        [[nodiscard]] friend constexpr integer operator-(integer a, integer b) noexcept { return integer(_as(a.u() - b.u(), For), unsafe); }
        [[nodiscard]] friend constexpr integer operator*(integer a, integer b) noexcept { return integer(_as(a.u() * b.u(), For), unsafe); }
        [[nodiscard]] friend constexpr integer operator/(integer a, integer b) noexcept
        {
            if (b.underlying == 0) [[unlikely]]
            {
                if constexpr (IBuiltinIntegerSigned<For>)
                    if (a.underlying < 0)
                        return integer(std::numeric_limits<For>::lowest());

                return integer(std::numeric_limits<For>::max());
            }
            else [[likely]]
                return integer(_as(a.underlying / b.underlying, For));
        }
        [[nodiscard]] friend constexpr integer operator%(integer a, integer b) noexcept
        {
            if (b.underlying == 0) [[unlikely]]
                return a;
            else [[likely]]
                return integer(_as(a.underlying % b.underlying, For));
        }

        [[nodiscard]] constexpr integer operator~() const noexcept { return integer(_as(~this->u(), For), unsafe); }
        [[nodiscard]] friend constexpr integer operator&(integer a, integer b) noexcept { return integer(_as(a.u() & b.u(), For), unsafe); }
        [[nodiscard]] friend constexpr integer operator|(integer a, integer b) noexcept { return integer(_as(a.u() | b.u(), For), unsafe); }
        [[nodiscard]] friend constexpr integer operator^(integer a, integer b) noexcept { return integer(_as(a.u() ^ b.u(), For), unsafe); }
        [[nodiscard]] friend constexpr integer operator<<(integer a, integer b) noexcept
        {
            if constexpr (IBuiltinIntegerSigned<For>)
                if (b.underlying < 0) [[unlikely]]
                    return integer(_as(a.u() >> _as((-std::max(b.underlying, _as(-integer<For>::fixed_width<For>() + _as(1, For), For))), unsigned_t), For), unsafe) &
                        integer(_as(-_as(b.underlying > -integer<For>::fixed_width<For>(), signed_t), For), unsafe);

            return integer(_as(a.u() << std::min(b.u(), _as(integer<For>::fixed_width<unsigned_t>() - _as(1, unsigned_t), unsigned_t)), For), unsafe) &
                integer(_as(-_as(b.underlying < integer<For>::fixed_width<For>(), signed_t), For), unsafe);
        }
        [[nodiscard]] friend constexpr integer operator>>(integer a, integer b) noexcept
        {
            if constexpr (IBuiltinIntegerSigned<For>)
                if (b.underlying < 0) [[unlikely]]
                    return integer(_as(a.u() << _as((-std::max(b.underlying, _as(-integer<For>::fixed_width<For>() + _as(1, For), For))), unsigned_t), For), unsafe) &
                        integer(_as(-_as(b.underlying > -integer<For>::fixed_width<For>(), signed_t), For), unsafe);

            return integer(_as(a.u() >> std::min(b.u(), _as(integer<For>::fixed_width<unsigned_t>() - _as(1, unsigned_t), unsigned_t)), For), unsafe) &
                integer(_as(-_as(b.underlying < integer<For>::fixed_width<For>(), signed_t), For), unsafe);
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
