#pragma once

#include <concepts>
#include <cstdint>
#include <limits>
#include <print>
#include <source_location>
#include <stdfloat>
#include <type_traits>
#include <utility>

#include <CompilerWarnings.h>

struct unsafe
{ };

/// @def _inline_always
/// @brief Force inline a function.
#define _inline_always [[gnu::always_inline]] inline
/// @def _inline_never
/// @brief Force noinline a function.
#define _inline_never [[gnu::noinline]]
/// @def _weak
/// @brief Mark a function as weak.
#define _weak [[gnu::weak]]
/// @def _pure
/// @brief Mark a function as pure.
#define _pure [[gnu::pure]]
/// @def _const
/// @brief Mark a function as const.
#define _const [[gnu::const]]
/// @def _restrict
/// @brief Mark a parameter (or this) as restrict.
#define _restrict __restrict__
/// @def _pack(align)
/// @brief Pack a structure to `align` bytes.
#define _pack(align) _clPragma_fwd(pack(align))
/// @def _packed
/// @brief Pack a structure to the smallest possible alignment.
#define _packed [[gnu::packed]]

#define _as(T, ...) static_cast<T>(__VA_ARGS__)
#define _asd(T, ...) dynamic_cast<T>(__VA_ARGS__)
#define _asc(T, ...) const_cast<T>(__VA_ARGS__)
#define _asr(T, ...) reinterpret_cast<T>(__VA_ARGS__)
#define _asi(T, ...) ::sys::numeric_cast<T>(__VA_ARGS__, unsafe())

/// @def _throw(value)
/// @brief Logs a source location, and throws the value of the expression `value`.
#define _throw(value)                                                                                                                                                           \
    do                                                                                                                                                                          \
    {                                                                                                                                                                           \
        std::source_location __srcLoc = std::source_location::current();                                                                                                        \
        _push_nowarn(_clWarn_use_after_free);                                                                                                                                   \
        std::println(stderr, "In function `{}` at \"{}:{}:{}\" - Throwing `{}`.", __srcLoc.function_name(), __srcLoc.file_name(), int(__srcLoc.line()), int(__srcLoc.column()), \
                     typeid(decltype(value)).name());                                                                                                                           \
        _pop_nowarn();                                                                                                                                                          \
        throw(value);                                                                                                                                                           \
    }                                                                                                                                                                           \
    while (false)

/// @def _fence_value_return(val, retCond)
/// @brief Return `val` if `retCond` is true.
#define _fence_value_return(val, retCond) \
    if (retCond)                          \
        return val;
/// @def _fence_value_co_return(val, retCond)
/// @brief Coroutine-return `val` if `retCond` is true.
#define _fence_value_co_return(val, retCond) \
    if (retCond)                             \
        co_return val;
/// @def _fence_contract_enforce(cond)
/// @brief Enforce a contract, throwing a `contract_violation_exception` if `cond` is false.
#define _fence_contract_enforce(cond)                                                                                      \
    do                                                                                                                     \
    {                                                                                                                      \
        const bool __expr = cond;                                                                                          \
        if (!__expr)                                                                                                       \
            _throw(::sys::contract_violation_exception("Contract violated, condition `" #cond "` evaluated to `false`.")); \
        [[assume(__expr)]];                                                                                                \
    }                                                                                                                      \
    while (false)

namespace sys
{
    template <typename T>
    concept INumberUnderlying = std::integral<T> || std::floating_point<T>;
    template <typename T, typename Than>
    concept IStrictlyWiderUnderlying = std::integral<T> && std::integral<Than> && std::cmp_less_equal(std::numeric_limits<T>::lowest(), std::numeric_limits<Than>::lowest()) &&
        std::cmp_greater_equal(std::numeric_limits<T>::max(), std::numeric_limits<Than>::max());
    template <typename T, typename Than>
    concept IStrictlyNarrowerUnderlying = IStrictlyWiderUnderlying<Than, T>;

    template <typename T, typename... WithSignednessOf>
    using type_int_with_signedness_underlying = std::conditional_t<(std::is_signed_v<WithSignednessOf> || ...), std::make_signed_t<T>, std::make_unsigned_t<T>>;

    template <typename T, typename... Rest>
    struct type_smallest_of
    {
        using RemainderType = type_smallest_of<Rest...>::Type;
        using Type = std::conditional_t<(sizeof(RemainderType) > sizeof(T)), T, RemainderType>;
    };
    template <typename T>
    struct type_smallest_of<T>
    {
        using Type = T;
    };

    template <typename T, typename... Rest>
    struct type_largest_of
    {
        using RemainderType = type_largest_of<Rest...>::Type;
        using Type = std::conditional_t<(sizeof(RemainderType) < sizeof(T)), T, RemainderType>;
    };
    template <typename T>
    struct type_largest_of<T>
    {
        using Type = T;
    };

    template <typename T, typename U = void>
    concept IEnumerable = std::is_array_v<T> || requires(T range, std::remove_cvref_t<decltype(range.begin())> it) {
        range.begin();
        range.end();

        range.begin() != range.end();
        ++it;

        requires ((std::same_as<U, void> && requires { *range.begin(); }) || std::convertible_to<decltype(*range.begin()), std::add_lvalue_reference_t<std::add_const_t<U>>>);
    } || requires(T range, std::remove_cvref_t<decltype(begin(range))> it) {
        begin(range);
        end(range);

        begin(range) != end(range);
        ++it;

        requires ((std::same_as<U, void> && requires { *begin(range); }) || std::convertible_to<decltype(*begin(range)), std::add_lvalue_reference_t<std::add_const_t<U>>>);
    };

    template <typename Functor, typename ReturnType, typename... Args>
    struct type_is_functor : std::is_invocable<ReturnType, Functor, Args...>
    { };
    template <typename Functor, typename ReturnType, typename... Args>
    struct type_is_functor<Functor, ReturnType(Args...)> : type_is_functor<Functor, ReturnType, Args...>
    { };

    template <typename Functor, typename Signature>
    concept IFunc = type_is_functor<Functor, Signature>::value;

    template <typename T, typename From>
    using type_with_qual_ref_from =
        std::conditional_t<std::is_reference_v<From>,
                           std::conditional_t<std::is_const_v<From>,
                                              std::conditional_t<std::is_volatile_v<From>, std::add_const_t<std::add_volatile_t<std::add_lvalue_reference_t<T>>>,
                                                                 std::add_const_t<std::add_lvalue_reference_t<T>>>,
                                              std::conditional_t<std::is_volatile_v<From>, std::add_volatile_t<std::add_lvalue_reference_t<T>>, std::add_lvalue_reference_t<T>>>,
                           std::conditional_t<std::is_const_v<From>, std::conditional_t<std::is_volatile_v<From>, std::add_const_t<std::add_volatile_t<T>>, std::add_const_t<T>>,
                                              std::conditional_t<std::is_volatile_v<From>, std::add_volatile_t<T>, T>>>;

    template <std::integral WithWidth>
    struct _packed alignas(WithWidth) integer
    {
        using Underlying = WithWidth;

        WithWidth underlying;

        constexpr integer() noexcept = default;
        template <IStrictlyNarrowerUnderlying<WithWidth> T>
        constexpr integer(T t) noexcept : underlying(t)
        { }
        template <std::integral T>
        constexpr integer(T t) noexcept : underlying(WithWidth(T(t) & T(std::numeric_limits<WithWidth>::max())))
        { }
        template <IStrictlyNarrowerUnderlying<WithWidth> T>
        constexpr integer(const integer<T>& t) noexcept : underlying(t.underlying)
        { }
        template <std::floating_point T>
        constexpr integer(T t) noexcept : underlying(static_cast<WithWidth>(t))
        { }

        template <std::integral T>
        constexpr explicit operator T() const noexcept
        {
            return T((typename type_largest_of<T, WithWidth>::Type(this->underlying)) & (typename type_largest_of<T, WithWidth>::Type(std::numeric_limits<T>::max())));
        }
        template <std::floating_point T>
        constexpr explicit operator T() const noexcept
        {
            return T(this->underlying);
        }
        constexpr WithWidth operator+() const noexcept
        {
            return this->underlying;
        }

        constexpr explicit operator bool() const noexcept
        {
            return this->underlying;
        }
        constexpr bool operator!() const noexcept
        {
            return !this->underlying;
        }
        template <std::integral Other>
        constexpr friend bool operator==(const sys::integer<WithWidth>& a, const sys::integer<Other>& b) noexcept
        {
            return std::cmp_equal(a.underlying, b.underlying);
        }
        template <std::integral Other>
        constexpr friend int operator<=>(const sys::integer<WithWidth>& a, const sys::integer<Other>& b) noexcept
        {
            return -int(std::cmp_less(a.underlying, b.underlying)) + int(std::cmp_greater(a.underlying, b.underlying));
        }
        template <std::integral Other>
        constexpr friend bool operator&&(const sys::integer<WithWidth>& a, const sys::integer<Other>& b) noexcept
        {
            return a.underlying && b.underlying;
        }
        template <std::integral Other>
        constexpr friend bool operator||(const sys::integer<WithWidth>& a, const sys::integer<Other>& b) noexcept
        {
            return a.underlying || b.underlying;
        }

        constexpr integer& operator++() noexcept
        {
            this->underlying = WithWidth(std::make_unsigned_t<WithWidth>(this->underlying) + std::make_unsigned_t<WithWidth>(1));
            return *this;
        }
        constexpr integer& operator--() noexcept
        {
            this->underlying = WithWidth(std::make_unsigned_t<WithWidth>(this->underlying) - std::make_unsigned_t<WithWidth>(1));
            return *this;
        }
        constexpr integer operator++(int) noexcept
        {
            integer ret = *this;
            this->underlying = WithWidth(std::make_unsigned_t<WithWidth>(this->underlying) + std::make_unsigned_t<WithWidth>(1));
            return ret;
        }
        constexpr integer operator--(int) noexcept
        {
            integer ret = *this;
            this->underlying = WithWidth(std::make_unsigned_t<WithWidth>(this->underlying) - std::make_unsigned_t<WithWidth>(1));
            return ret;
        }

        constexpr integer operator-() const noexcept
        {
            union
            {
                WithWidth i;
                std::make_unsigned_t<WithWidth> u;
            } ret { .u = 0 };
            ret.u -= std::make_unsigned_t<WithWidth>(this->underlying);
            return ret.i;
        }
        template <std::integral Other>
        friend constexpr integer<typename type_largest_of<WithWidth, Other>::Type> operator+(const integer<WithWidth>& a, const integer<Other>& b) noexcept
        {
            union
            {
                WithWidth i;
                std::make_unsigned_t<WithWidth> u;
            } ret { .u = a.underlying };
            ret.u += std::make_unsigned_t<WithWidth>(b.underlying);
            return ret.i;
        }
        template <std::integral Other>
        friend constexpr integer<typename type_largest_of<WithWidth, Other>::Type> operator-(const integer<WithWidth>& a, const integer<Other>& b) noexcept
        {
            union
            {
                WithWidth i;
                std::make_unsigned_t<WithWidth> u;
            } ret { .u = a.underlying };
            ret.u -= std::make_unsigned_t<WithWidth>(b.underlying);
            return ret.i;
        }
        template <std::integral Other>
        friend constexpr integer<typename type_largest_of<WithWidth, Other>::Type> operator*(const integer<WithWidth>& a, const integer<Other>& b) noexcept
        {
            union
            {
                WithWidth i;
                std::make_unsigned_t<WithWidth> u;
            } ret { .u = a.underlying };
            ret.u *= std::make_unsigned_t<WithWidth>(b.underlying);
            return ret.i;
        }
        // todo: division, mod

        constexpr integer operator~() const noexcept
        {
            return WithWidth(~std::make_unsigned_t<WithWidth>(this->underlying));
        }
        template <std::integral Other>
        friend constexpr integer<typename type_largest_of<WithWidth, Other>::Type> operator&(const integer<WithWidth>& a, const integer<Other>& b) noexcept
        {
            return typename type_largest_of<WithWidth, Other>::Type(std::make_unsigned_t<typename type_largest_of<WithWidth, Other>::Type>(a.underlying) &
                                                                    std::make_unsigned_t<typename type_largest_of<WithWidth, Other>::Type>(b.underlying));
        }
        template <std::integral Other>
        friend constexpr integer<typename type_largest_of<WithWidth, Other>::Type> operator|(const integer<WithWidth>& a, const integer<Other>& b) noexcept
        {
            return typename type_largest_of<WithWidth, Other>::Type(std::make_unsigned_t<typename type_largest_of<WithWidth, Other>::Type>(a.underlying) |
                                                                    std::make_unsigned_t<typename type_largest_of<WithWidth, Other>::Type>(b.underlying));
        }
        template <std::integral Other>
        friend constexpr integer<typename type_largest_of<WithWidth, Other>::Type> operator^(const integer<WithWidth>& a, const integer<Other>& b) noexcept
        {
            return typename type_largest_of<WithWidth, Other>::Type(std::make_unsigned_t<typename type_largest_of<WithWidth, Other>::Type>(a.underlying) ^
                                                                    std::make_unsigned_t<typename type_largest_of<WithWidth, Other>::Type>(b.underlying));
        }
        template <std::unsigned_integral Other>
        friend constexpr integer operator<<(const integer<WithWidth>& a, const Other& b) noexcept
        {
            return integer(WithWidth(std::make_unsigned_t<WithWidth>(a.underlying) << b));
        }
        template <std::unsigned_integral Other>
        friend constexpr integer operator>>(const integer<WithWidth>& a, const Other& b) noexcept
        {
            return integer(WithWidth(std::make_unsigned_t<WithWidth>(a.underlying) >> b));
        }
        template <std::unsigned_integral Other>
        friend constexpr integer operator<<(const integer<WithWidth>& a, const integer<Other>& b) noexcept
        {
            return integer(WithWidth(std::make_unsigned_t<WithWidth>(a.underlying) << b.underlying));
        }
        template <std::unsigned_integral Other>
        friend constexpr integer operator>>(const integer<WithWidth>& a, const integer<Other>& b) noexcept
        {
            return integer(WithWidth(std::make_unsigned_t<WithWidth>(a.underlying) >> b.underlying));
        }

        template <std::integral T>
        constexpr integer& operator=(const integer<T>& other) noexcept
        {
            *this = other;
            return *this;
        }
        template <std::integral T>
        constexpr integer& operator+=(const integer<T>& other) noexcept
        {
            *this = *this + other;
            return *this;
        }
        template <std::integral T>
        constexpr integer& operator-=(const integer<T>& other) noexcept
        {
            *this = *this - other;
            return *this;
        }
        template <std::integral T>
        constexpr integer& operator*=(const integer<T>& other) noexcept
        {
            *this = *this * other;
            return *this;
        }
        template <std::integral T>
        constexpr integer& operator&=(const integer<T>& other) noexcept
        {
            *this = *this & other;
            return *this;
        }
        template <std::integral T>
        constexpr integer& operator|=(const integer<T>& other) noexcept
        {
            *this = *this | other;
            return *this;
        }
        template <std::integral T>
        constexpr integer& operator^=(const integer<T>& other) noexcept
        {
            *this = *this ^ other;
            return *this;
        }
        template <std::unsigned_integral T>
        constexpr integer& operator<<=(const integer<T>& other) noexcept
        {
            *this = *this << other;
            return *this;
        }
        template <std::unsigned_integral T>
        constexpr integer& operator>>=(const integer<T>& other) noexcept
        {
            *this = *this >> other;
            return *this;
        }
        // todo: diveq, modeq
    };
} // namespace sys

template <std::integral T, sys::INumberUnderlying U>
constexpr bool operator==(const sys::integer<T>& a, U b) noexcept
{
    return std::cmp_equal(a.underlying, b);
}
template <sys::INumberUnderlying T, std::integral U>
constexpr bool operator==(U a, const sys::integer<T>& b) noexcept
{
    return std::cmp_equal(a, b.underlying);
}
template <std::integral T, sys::INumberUnderlying U>
constexpr int operator<=>(const sys::integer<T>& a, U b) noexcept
{
    return -int(std::cmp_less(a.underlying, b)) + int(std::cmp_greater(a.underlying, b));
}
template <sys::INumberUnderlying T, std::integral U>
constexpr int operator<=>(U a, const sys::integer<T>& b) noexcept
{
    return -int(std::cmp_less(a, b.underlying)) + int(std::cmp_greater(a, b.underlying));
}

using byte = unsigned char;
using sbyte = signed char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;
using llong = long long;
using ullong = unsigned long long;

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

_push_nowarn(_clWarn_literal_suffix);
// clang-format off: C++23 -- no space b/w "" and literal suffix.
constexpr i8 operator""i8(ullong lit) noexcept
{
    return i8(i8::Underlying(lit));
}
constexpr i16 operator""i16(ullong lit) noexcept
{
    return i16(i16::Underlying(lit));
}
constexpr i32 operator""i32(ullong lit) noexcept
{
    return i32(i32::Underlying(lit));
}
constexpr i64 operator""i64(ullong lit) noexcept
{
    return i64(i64::Underlying(lit));
}
constexpr u8 operator""u8(ullong lit) noexcept
{
    return u8(u8::Underlying(lit));
}
constexpr u16 operator""u16(ullong lit) noexcept
{
    return u16(u16::Underlying(lit));
}
constexpr u32 operator""u32(ullong lit) noexcept
{
    return u32(u32::Underlying(lit));
}
constexpr u64 operator""u64(ullong lit) noexcept
{
    return u64(u64::Underlying(lit));
}
constexpr ssz operator""zz(ullong lit) noexcept
{
    return ssz(ssz::Underlying(lit));
}
constexpr sz operator""uzz(ullong lit) noexcept
{
    return sz(sz::Underlying(lit));
}
// clang-format on
_pop_nowarn();

#if !__STDCPP_FLOAT32_T__
using f32 = std::float32_t;
#endif
#if !__STDCPP_FLOAT64_T__
using f64 = std::float64_t;
#endif
