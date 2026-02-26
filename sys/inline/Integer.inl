#pragma once

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
    constexpr /* NOLINT(misc-use-internal-linkage) */ To bnumeric_cast(const From value, unsafe) noexcept
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
    constexpr /* NOLINT(misc-use-internal-linkage) */ To bnumeric_cast(const From value, unsafe) noexcept
    {
        if (!std::isfinite(value) || value <= _as(From, std::numeric_limits<To>::lowest())) [[unlikely]]
            return std::numeric_limits<To>::lowest();
        else if (value >= _as(From, std::numeric_limits<To>::max())) [[unlikely]]
            return std::numeric_limits<To>::max();
        else [[likely]]
            return To(value);
    }

    template <sys::IBuiltinIntegerSigned T>
    consteval /* NOLINT(misc-use-internal-linkage) */ T bsentinel()
    {
        return std::numeric_limits<T>::lowest();
    }
    template <sys::IBuiltinIntegerUnsigned T>
    consteval /* NOLINT(misc-use-internal-linkage) */ T bsentinel()
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
    struct alignas(For) integer
    {
    private:
        using signed_t = std::make_signed_t<For>;
        using unsigned_t = std::make_unsigned_t<For>;

        For underlying = 0;

        [[nodiscard]] constexpr unsigned_t u() const noexcept { return std::bit_cast<unsigned_t>(**this); }
    public:
        using underlying_type = For;

        [[nodiscard]] static consteval bool is_signed() { return std::is_signed_v<For>; }

        [[nodiscard]] static consteval integer highest() { return std::numeric_limits<For>::max(); }
        [[nodiscard]] static consteval integer lowest() { return std::numeric_limits<For>::lowest(); }
        [[nodiscard]] static consteval integer ones() { return _as(For, ~_as(For, 0)); }
        [[nodiscard]] static consteval integer sentinel() { return sys::bsentinel<For>(); }

        constexpr integer() noexcept = default;
        template <IBuiltinIntegerCanHold<For> T>
        constexpr integer(T v) noexcept : underlying(_as(For, v)) // NOLINT(hicpp-explicit-conversions)
        { }
        template <typename T>
        requires ((!IBuiltinIntegerCanHold<T, For> && sys::IBuiltinInteger<T>) || sys::IBuiltinFloatingPoint<T>)
        constexpr explicit integer(T v) noexcept : underlying(bnumeric_cast<For>(v, unsafe()))
        { }
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(T v, unsafe) noexcept : underlying(std::bit_cast<For>(_as(unsigned_t, std::bit_cast<std::make_unsigned_t<T>>(v))))
        { }
        template <sys::IBuiltinInteger T>
        constexpr explicit integer(integer<T> v) noexcept : integer(*v)
        { }
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
        template <sys::IBuiltinInteger T>
        [[nodiscard]] constexpr explicit operator T() const noexcept
        {
            return bnumeric_cast<T>(**this, unsafe());
        }
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        [[nodiscard]] constexpr operator For() const noexcept { return **this; }
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
        /// @note Be warned that the negation of `integer<...>::lowest()` is not UB but instead `integer<...>::highest()`.
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
consteval /* NOLINT(misc-use-internal-linkage) */ i8 operator""_i8(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least8_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least8_t>::min()))
        throw std::overflow_error("Literal too large for `i8`.");

    return { _as(int_least8_t, lit) };
}
/// @brief Literal suffix for `i16`.
consteval /* NOLINT(misc-use-internal-linkage) */ i16 operator""_i16(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least16_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least16_t>::min()))
        throw std::overflow_error("Literal too large for `i16`.");

    return { _as(int_least16_t, lit) };
}
/// @brief Literal suffix for `i32`.
consteval /* NOLINT(misc-use-internal-linkage) */ i32 operator""_i32(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least32_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least32_t>::min()))
        throw std::overflow_error("Literal too large for `i32`.");

    return { _as(int_least32_t, lit) };
}
/// @brief Literal suffix for `i64`.
consteval /* NOLINT(misc-use-internal-linkage) */ i64 operator""_i64(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<int_least64_t>::max()) || std::cmp_less(lit, std::numeric_limits<int_least64_t>::min()))
        throw std::overflow_error("Literal too large for `i64`.");

    return { _as(int_least64_t, lit) };
}
/// @brief Literal suffix for `u8`.
consteval /* NOLINT(misc-use-internal-linkage) */ u8 operator""_u8(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least8_t>::max()))
        throw std::overflow_error("Literal too large for `u8`.");

    return { _as(uint_least8_t, lit) };
}
/// @brief Literal suffix for `u16`.
consteval /* NOLINT(misc-use-internal-linkage) */ u16 operator""_u16(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least16_t>::max()))
        throw std::overflow_error("Literal too large for `u16`.");

    return { _as(uint_least16_t, lit) };
}
/// @brief Literal suffix for `u32`.
consteval /* NOLINT(misc-use-internal-linkage) */ u32 operator""_u32(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least32_t>::max()))
        throw std::overflow_error("Literal too large for `u32`.");

    return { _as(uint_least32_t, lit) };
}
/// @brief Literal suffix for `u64`.
consteval /* NOLINT(misc-use-internal-linkage) */ u64 operator""_u64(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<uint_least64_t>::max()))
        throw std::overflow_error("Literal too large for `u64`.");

    return { _as(uint_least64_t, lit) };
}
/// @brief Literal suffix for `ssz`.
consteval /* NOLINT(misc-use-internal-linkage) */ ssz operator""_z(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<ptrdiff_t>::max()) || std::cmp_less(lit, std::numeric_limits<ptrdiff_t>::min()))
        throw std::overflow_error("Literal too large for `ssz`.");

    return { _as(ptrdiff_t, lit) };
}
/// @brief Literal suffix for `sz`.
consteval /* NOLINT(misc-use-internal-linkage) */ sz operator""_uz(ullong lit) noexcept
{
    if (std::cmp_greater(lit, std::numeric_limits<size_t>::max()))
        throw std::overflow_error("Literal too large for `sz`.");

    return { _as(size_t, lit) };
}
// clang-format on
// NOLINTEND(bugprone-exception-escape)

/// @}
