#pragma once

/// @file Integer.inl

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
#include <Traits.h>

/// @defgroup builtin_integers Built-in Integer Types
/// @brief Abbreviated built-in integer types.
/// @note Pass `byval`.
/// @{

using byte = unsigned char; /**< Unsigned byte type. */             // NOLINT()
using sbyte = signed char; /**< Signed byte type. */                // NOLINT()
using ushort = unsigned short; /**< Unsigned short type. */         // NOLINT(google-runtime-int)
using uint = unsigned int; /**< Unsigned int type. */               // NOLINT()
using ulong = unsigned long; /**< Unsigned long type. */            // NOLINT(google-runtime-int)
using llong = long long; /**< Signed long long type. */             // NOLINT(google-runtime-int)
using ullong = unsigned long long; /**< Unsigned long long type. */ // NOLINT(google-runtime-int)

/// @}

namespace sys
{
    /// @brief Saturating integer-to-integer cast.
    template <sys::IBuiltinInteger To, sys::IBuiltinInteger From>
    constexpr To bnumeric_cast(const From value, unsafe) noexcept // NOLINT(misc-use-internal-linkage)
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
    template <sys::IBuiltinInteger To, sys::IBuiltinFloatingPoint From>
    constexpr To bnumeric_cast(const From value, unsafe) noexcept // NOLINT(misc-use-internal-linkage)
    {
        if (!std::isfinite(value) || value <= _as(From, std::numeric_limits<To>::lowest())) [[unlikely]]
            return std::numeric_limits<To>::lowest();
        else if (value >= _as(From, std::numeric_limits<To>::max())) [[unlikely]]
            return std::numeric_limits<To>::max();
        else [[likely]]
            return To(value);
    }

    /// @brief Returns an opinionated sentinel value for `T`.
    template <sys::IBuiltinIntegerSigned T>
    consteval T bsentinel() // NOLINT(misc-use-internal-linkage)
    {
        return std::numeric_limits<T>::lowest();
    }
    /// @brief Returns an opinionated sentinel value for `T`.
    template <sys::IBuiltinIntegerUnsigned T>
    consteval T bsentinel() // NOLINT(misc-use-internal-linkage)
    {
        return std::numeric_limits<T>::max();
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
    struct alignas(For) integer final
    {
    private:
        using signed_t = std::make_signed_t<For>;
        using unsigned_t = std::make_unsigned_t<For>;

        For underlying = 0;

        [[nodiscard]] constexpr unsigned_t u() const noexcept { return std::bit_cast<unsigned_t>(**this); }
    public:
        using underlying_type = For; /**< Identity of `For`. */

        [[nodiscard]] static consteval bool is_signed() { return std::is_signed_v<For>; } /**< Whether `underlying_type` is signed. */

        [[nodiscard]] static consteval integer highest() { return std::numeric_limits<For>::max(); }   /**< Maximum value of `underlying_type`. */
        [[nodiscard]] static consteval integer lowest() { return std::numeric_limits<For>::lowest(); } /**< Minimum value of `underlying_type`. */
        [[nodiscard]] static consteval integer ones() { return _as(For, ~_as(For, 0)); }               /**< All bits set to `1`. */
        [[nodiscard]] static consteval integer sentinel() { return sys::bsentinel<For>(); }            /**< Opinionated sentinel value for `underlying_type`. */

        constexpr integer() noexcept = default;
        /// @brief Construct from narrower-bounded `T`.
        template <IBuiltinIntegerCanHold<For> T>
        constexpr integer(T v) noexcept : underlying(_as(For, v)) // NOLINT(hicpp-explicit-conversions)
        { }
        /// @brief Saturating construction from any numerical `T`.
        template <typename T>
        requires ((!IBuiltinIntegerCanHold<T, For> && sys::IBuiltinInteger<T>) || sys::IBuiltinFloatingPoint<T>)
        constexpr explicit integer(T v) noexcept : underlying(bnumeric_cast<For>(v, unsafe()))
        { }
        /// @brief Truncating construction from any built-in integer `T`.
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(T v, unsafe) noexcept : underlying(std::bit_cast<For>(_as(unsigned_t, std::bit_cast<std::make_unsigned_t<T>>(v))))
        { }
        /// @brief Saturating copy construction from any other `sys::integer<T>`.
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(integer<T> v) noexcept : integer(*v)
        { }
        /// @brief Truncating copy construction from any other `sys::integer<T>`.
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(integer<T> v, unsafe) noexcept : integer(*v, unsafe())
        { }
        /// @brief Copyable.
        constexpr integer(const integer& v) noexcept = default;
        /// @brief Moveable.
        constexpr integer(integer&& v) noexcept = default;
        constexpr ~integer() noexcept = default;

        /// @brief Assignment from narrower-bounded `T`.
        template <IBuiltinIntegerCanHold<For> T>
        constexpr integer& operator=(T v) noexcept
        {
            this->underlying = _as(For, v);
            return *this;
        }
        /// @brief Copy-assignable.
        constexpr integer& operator=(const integer& other) noexcept = default;
        /// @brief Move-assignable.
        constexpr integer& operator=(integer&& other) noexcept = default;

        /// @brief Underlying value of built-in type.
        [[nodiscard]] constexpr For operator*() const noexcept { return this->underlying; }
        /// @brief Underlying value of built-in type.
        [[nodiscard]] constexpr For& operator*() noexcept { return this->underlying; }

        /// @brief Explicit saturating conversion to any built-in integer `T`.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return bnumeric_cast<T>(**this, unsafe());
        }
        /// @brief Underlying value of built-in type.
        [[nodiscard]] constexpr operator For() const noexcept { return **this; } // NOLINT(hicpp-explicit-conversions)
        /// @brief Explicit conversion to any floating-point `T`.
        template <sys::IBuiltinFloatingPoint T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return _as(T, **this);
        }

        /// @brief Explicit conversion to `bool`.
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return **this; }
        /// @brief Boolean negation.
        [[nodiscard]] constexpr bool operator!() const noexcept { return !**this; }

        /// @brief Transparent equality comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator==(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_equal(*a, *b);
        }
        /// @brief Transparent equality comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator==(integer<For> a, T b) noexcept
        {
            return std::cmp_equal(*a, b);
        }
        /// @brief Transparent equality comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator==(For a, integer<T> b) noexcept
        {
            return std::cmp_equal(a, *b);
        }
        /// @brief Transparent less-than comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_less(*a, *b);
        }
        /// @brief Transparent less-than comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<(integer<For> a, T b) noexcept
        {
            return std::cmp_less(*a, b);
        }
        /// @brief Transparent less-than comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<(For a, integer<T> b) noexcept
        {
            return std::cmp_less(a, *b);
        }
        /// @brief Transparent less-equal comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<=(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_less_equal(*a, *b);
        }
        /// @brief Transparent less-equal comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<=(integer<For> a, T b) noexcept
        {
            return std::cmp_less_equal(*a, b);
        }
        /// @brief Transparent less-equal comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator<=(For a, integer<T> b) noexcept
        {
            return std::cmp_less_equal(a, *b);
        }
        /// @brief Transparent greater-than comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_greater(*a, *b);
        }
        /// @brief Transparent greater-than comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>(integer<For> a, T b) noexcept
        {
            return std::cmp_greater(*a, b);
        }
        /// @brief Transparent greater-than comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>(For a, integer<T> b) noexcept
        {
            return std::cmp_greater(a, *b);
        }
        /// @brief Transparent greater-equal comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>=(integer<For> a, integer<T> b) noexcept
        {
            return std::cmp_greater_equal(*a, *b);
        }
        /// @brief Transparent greater-equal comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>=(integer<For> a, T b) noexcept
        {
            return std::cmp_greater_equal(*a, b);
        }
        /// @brief Transparent greater-equal comparison.
        template <sys::IBuiltinInteger T>
        [[nodiscard]] friend constexpr bool operator>=(For a, integer<T> b) noexcept
        {
            return std::cmp_greater_equal(a, *b);
        }

        /// @brief Pre-increment.
        constexpr integer& operator++() noexcept { return (*this = integer(this->u() + _as(unsigned_t, 1), unsafe())); }
        /// @brief Pre-decrement.
        constexpr integer& operator--() noexcept { return (*this = integer(this->u() - _as(unsigned_t, 1), unsafe())); }
        /// @brief Post-increment.
        constexpr integer operator++(int) noexcept
        {
            integer ret = *this;
            *this = integer(this->u() + _as(unsigned_t, 1), unsafe());
            return ret;
        }
        /// @brief Post-decrement.
        constexpr integer operator--(int) noexcept
        {
            integer ret = *this;
            *this = integer(this->u() - _as(unsigned_t, 1), unsafe());
            return ret;
        }

        /// @brief Unary plus.
        [[nodiscard]] constexpr integer operator+() const noexcept { return *this; }
        /// @brief Unary minus.
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
        /// @brief Integer addition.
        [[nodiscard]] friend constexpr integer operator+(integer a, integer b) noexcept { return integer(a.u() + b.u(), unsafe()); }
        /// @brief Integer subtraction.
        [[nodiscard]] friend constexpr integer operator-(integer a, integer b) noexcept { return integer(a.u() - b.u(), unsafe()); }
        /// @brief Integer multiplication.
        [[nodiscard]] friend constexpr integer operator*(integer a, integer b) noexcept { return integer(a.u() * b.u(), unsafe()); }
        /// @brief Integer division.
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
        /// @brief Integer remainder.
        [[nodiscard]] friend constexpr integer operator%(integer a, integer b) noexcept
        {
            if (*b == 0) [[unlikely]]
                return a;
            else [[likely]]
                return integer(_as(For, *a % *b));
        }

        /// @brief Bitwise not.
        [[nodiscard]] constexpr integer operator~() const noexcept { return integer(~this->u(), unsafe()); }
        /// @brief Bitwise and.
        [[nodiscard]] friend constexpr integer operator&(integer a, integer b) noexcept { return integer(a.u() & b.u(), unsafe()); }
        /// @brief Bitwise or.
        [[nodiscard]] friend constexpr integer operator|(integer a, integer b) noexcept { return integer(a.u() | b.u(), unsafe()); }
        /// @brief Bitwise xor.
        [[nodiscard]] friend constexpr integer operator^(integer a, integer b) noexcept { return integer(a.u() ^ b.u(), unsafe()); }
        /// @brief Bitwise logical left shift.
        [[nodiscard]] friend constexpr integer operator<<(integer a, integer b) noexcept
        {
            if constexpr (std::is_signed_v<For>)
                if (*b < 0) [[unlikely]]
                    return integer(a.u() >> (-b).u(), unsafe()); // Note: `integer<...>::operator-()` produces signed max for negation of signed min, so safe.

            return integer(a.u() << b.u(), unsafe());
        }
        /// @brief Bitwise logical right shift.
        [[nodiscard]] friend constexpr integer operator>>(integer a, integer b) noexcept
        {
            if constexpr (std::is_signed_v<For>)
                if (*b < 0) [[unlikely]]
                    return integer(a.u() << (-b).u(), unsafe()); // Note: `integer<...>::operator-()` produces signed max for negation of signed min, so safe.

            return integer(a.u() >> b.u(), unsafe());
        }

        /// @brief Add-assign.
        constexpr integer& operator+=(const integer& other) noexcept { return (*this = *this + other); }
        /// @brief Sub-assign.
        constexpr integer& operator-=(const integer& other) noexcept { return (*this = *this - other); }
        /// @brief Mul-assign.
        constexpr integer& operator*=(const integer& other) noexcept { return (*this = *this * other); }
        /// @brief Div-assign.
        constexpr integer& operator/=(const integer& other) noexcept { return (*this = *this / other); }
        /// @brief Mod-assign.
        constexpr integer& operator%=(const integer& other) noexcept { return (*this = *this % other); }
        /// @brief Bitand-assign.
        constexpr integer& operator&=(const integer& other) noexcept { return (*this = *this & other); }
        /// @brief Bitor-assign.
        constexpr integer& operator|=(const integer& other) noexcept { return (*this = *this | other); }
        /// @brief Bitxor-assign.
        constexpr integer& operator^=(const integer& other) noexcept { return (*this = *this ^ other); }
        /// @brief Bitlsh-assign.
        constexpr integer& operator<<=(const integer& other) noexcept { return (*this = *this << other); }
        /// @brief Bitrsh-assign.
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

using i8 = ::sys::integer<int_least8_t>;   /**< Safe signed 8-bit int type. */
using i16 = ::sys::integer<int_least16_t>; /**< Safe signed 16-bit int type. */
using i32 = ::sys::integer<int_least32_t>; /**< Safe signed 32-bit int type. */
using i64 = ::sys::integer<int_least64_t>; /**< Safe signed 64-bit int type. */

using u8 = ::sys::integer<uint_least8_t>;   /**< Safe unsigned 8-bit int type. */
using u16 = ::sys::integer<uint_least16_t>; /**< Safe unsigned 16-bit int type. */
using u32 = ::sys::integer<uint_least32_t>; /**< Safe unsigned 32-bit int type. */
using u64 = ::sys::integer<uint_least64_t>; /**< Safe unsigned 64-bit int type. */

using sz = ::sys::integer<size_t>;     /**< Safe unsigned size type. */
using ssz = ::sys::integer<ptrdiff_t>; /**< Safe signed size type. */

/// @}

/// @defgroup integer_literals Integer Literals
/// @brief Literal suffixes for `sys::integer<...>`.
/// @note Pass `byval`.
/// @{

// NOLINTBEGIN(bugprone-exception-escape)
// clang-format off: C++23 -- no space b/w "" and literal suffix.
/// @brief Literal suffix for `i8`.
consteval i8 operator""_i8(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least8_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least8_t>::min()))
        throw std::overflow_error("Literal too large for `i8`.");

    return { _as(int_least8_t, lit) };
}
/// @brief Literal suffix for `i16`.
consteval i16 operator""_i16(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least16_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least16_t>::min()))
        throw std::overflow_error("Literal too large for `i16`.");

    return { _as(int_least16_t, lit) };
}
/// @brief Literal suffix for `i32`.
consteval i32 operator""_i32(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least32_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least32_t>::min()))
        throw std::overflow_error("Literal too large for `i32`.");

    return { _as(int_least32_t, lit) };
}
/// @brief Literal suffix for `i64`.
consteval i64 operator""_i64(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least64_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least64_t>::min()))
        throw std::overflow_error("Literal too large for `i64`.");

    return { _as(int_least64_t, lit) };
}
/// @brief Literal suffix for `u8`.
consteval u8 operator""_u8(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least8_t>::max()))
        throw std::overflow_error("Literal too large for `u8`.");

    return { _as(uint_least8_t, lit) };
}
/// @brief Literal suffix for `u16`.
consteval u16 operator""_u16(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least16_t>::max()))
        throw std::overflow_error("Literal too large for `u16`.");

    return { _as(uint_least16_t, lit) };
}
/// @brief Literal suffix for `u32`.
consteval u32 operator""_u32(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least32_t>::max()))
        throw std::overflow_error("Literal too large for `u32`.");

    return { _as(uint_least32_t, lit) };
}
/// @brief Literal suffix for `u64`.
consteval u64 operator""_u64(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least64_t>::max()))
        throw std::overflow_error("Literal too large for `u64`.");

    return { _as(uint_least64_t, lit) };
}
/// @brief Literal suffix for `ssz`.
consteval ssz operator""_z(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<ptrdiff_t>::max()) || std::cmp_less(lit, std::numeric_limits<ptrdiff_t>::min()))
        throw std::overflow_error("Literal too large for `ssz`.");

    return { _as(ptrdiff_t, lit) };
}
/// @brief Literal suffix for `sz`.
consteval sz operator""_uz(ullong lit) noexcept // NOLINT(misc-use-internal-linkage)
{
    if (std::cmp_greater(lit, std::numeric_limits<size_t>::max()))
        throw std::overflow_error("Literal too large for `sz`.");

    return { _as(size_t, lit) };
}
// clang-format on
// NOLINTEND(bugprone-exception-escape)

/// @}
