#pragma once

#include <concepts>
#include <cstdint>
#include <limits>
#include <numeric>
#include <print>
#include <source_location>
#include <stdfloat>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include <CompilerWarnings.h>

struct unsafe_tag
{ };
constexpr unsafe_tag unsafe {};

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

#define _as(T, ...) static_cast<T>(__VA_ARGS__)
#define _asd(T, ...) dynamic_cast<T>(__VA_ARGS__)
#define _asc(T, ...) const_cast<T>(__VA_ARGS__)
#define _asr(T, ...) reinterpret_cast<T>(__VA_ARGS__)
#define _asi(T, ...) ::sys::numeric_cast<T>(__VA_ARGS__, unsafe)

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
/// @brief Enforce a contract, throwing a `ContractViolationException` if `cond` is false.
#define _fence_contract_enforce(cond)                                                                                    \
    do                                                                                                                   \
    {                                                                                                                    \
        const bool __expr = cond;                                                                                        \
        if (!__expr)                                                                                                     \
            _throw(::sys::ContractViolationException("Contract violated, condition `" #cond "` evaluated to `false`.")); \
        [[assume(__expr)]];                                                                                              \
    }                                                                                                                    \
    while (false)

namespace sys
{
    template <typename T>
    concept INumberUnderlying = std::integral<T> || std::floating_point<T>;
    template <typename T, typename Than>
    concept IStrictlyWiderUnderlying = std::cmp_less_equal(std::numeric_limits<T>::lowest(), std::numeric_limits<Than>::lowest()) &&
        std::cmp_greater_equal(std::numeric_limits<T>::max(), std::numeric_limits<Than>::max());
    template <typename T, typename Than>
    concept IStrictlyNarrowerUnderlying = IStrictlyWiderUnderlying<Than, T>;

    template <typename T, typename... WithSignednessOf>
    using TypeIntWithSignednessUnderlying = std::conditional_t<(std::is_signed_v<WithSignednessOf> || ...), std::make_signed_t<T>, std::make_unsigned_t<T>>;

    template <typename T, typename... Rest>
    struct TypeSmallestOf
    {
        using RemainderType = TypeSmallestOf<Rest...>;
        using Type = std::conditional_t<(sizeof(RemainderType) > sizeof(T)), T, RemainderType>;
    };
    template <typename T>
    struct TypeSmallestOf<T>
    {
        using Type = T;
    };

    template <typename T, typename... Rest>
    struct TypeLargestOf
    {
        using RemainderType = TypeLargestOf<Rest...>;
        using Type = std::conditional_t<(sizeof(RemainderType) < sizeof(T)), T, RemainderType>;
    };
    template <typename T>
    struct TypeLargestOf<T>
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
    struct TypeIsFunctor : std::is_invocable<ReturnType, Functor, Args...>
    { };
    template <typename Functor, typename ReturnType, typename... Args>
    struct TypeIsFunctor<Functor, ReturnType(Args...)> : TypeIsFunctor<Functor, ReturnType, Args...>
    { };

    template <typename Functor, typename Signature>
    concept IFunc = TypeIsFunctor<Functor, Signature>::value;

    template <typename T, typename From>
    using TypeWithQualRefFrom =
        std::conditional_t<std::is_reference_v<From>,
                           std::conditional_t<std::is_const_v<From>,
                                              std::conditional_t<std::is_volatile_v<From>, std::add_const_t<std::add_volatile_t<std::add_lvalue_reference_t<T>>>,
                                                                 std::add_const_t<std::add_lvalue_reference_t<T>>>,
                                              std::conditional_t<std::is_volatile_v<From>, std::add_volatile_t<std::add_lvalue_reference_t<T>>, std::add_lvalue_reference_t<T>>>,
                           std::conditional_t<std::is_const_v<From>, std::conditional_t<std::is_volatile_v<From>, std::add_const_t<std::add_volatile_t<T>>, std::add_const_t<T>>,
                                              std::conditional_t<std::is_volatile_v<From>, std::add_volatile_t<T>, T>>>;

#pragma pack(1)
    template <std::integral WithWidth>
    struct alignas(WithWidth) Integer
    {
        using UnderlyingType = WithWidth;

        constexpr Integer() = default;
        template <IStrictlyNarrowerUnderlying<WithWidth> T>
        constexpr Integer(T t) : value(t)
        { }

        template <std::integral T>
        constexpr operator T() const noexcept
        {
            return T(typename TypeLargestOf<T, WithWidth>::Type(this->value) & typename TypeLargestOf<T, WithWidth>::Type(std::numeric_limits<T>::max()));
        }
        template <std::floating_point T>
        constexpr operator T() const noexcept
        {
            return T(this->value);
        }
        constexpr WithWidth operator+() const noexcept
        {
            return this->value;
        }

        constexpr operator bool() const noexcept
        {
            return this->value;
        }
        constexpr bool operator!() const noexcept
        {
            return !this->value;
        }
        template <std::integral Other>
        friend constexpr int operator<=>(const Integer& a, const Integer<Other>& b) noexcept
        {
            return -int(std::cmp_less(a.value, b.value)) + int(std::cmp_greater(a.value, b.value));
        }
        template <std::integral Other>
        friend constexpr int operator&&(const Integer& a, const Integer<Other>& b) noexcept
        {
            return a.value && b.value;
        }
        template <std::integral Other>
        friend constexpr int operator||(const Integer& a, const Integer<Other>& b) noexcept
        {
            return a.value || b.value;
        }

        constexpr Integer& operator++() const noexcept
        {
            this->value = ++std::make_unsigned_t<WithWidth>(this->value);
            return *this;
        }
        constexpr Integer& operator--() const noexcept
        {
            this->value = --std::make_unsigned_t<WithWidth>(this->value);
            return *this;
        }
        constexpr Integer operator++(int) const noexcept
        {
            Integer ret = *this;
            this->value = ++std::make_unsigned_t<WithWidth>(this->value);
            return ret;
        }
        constexpr Integer operator--(int) const noexcept
        {
            Integer ret = *this;
            this->value = --std::make_unsigned_t<WithWidth>(this->value);
            return ret;
        }

        constexpr Integer operator-() const noexcept
        {
            union
            {
                WithWidth i;
                std::make_unsigned_t<WithWidth> u;
            } ret { .u = 0 };
            ret.u -= std::make_unsigned_t<WithWidth>(this->value);
            return ret.i;
        }
        template <std::integral Other>
        friend constexpr Integer<typename TypeLargestOf<WithWidth, Other>::Type> operator+(const Integer<WithWidth>& a, const Integer<Other>& b) noexcept
        {
            union
            {
                WithWidth i;
                std::make_unsigned_t<WithWidth> u;
            } ret { .u = a.value };
            ret.u += std::make_unsigned_t<WithWidth>(b.value);
            return ret.i;
        }
        template <std::integral Other>
        friend constexpr Integer<typename TypeLargestOf<WithWidth, Other>::Type> operator-(const Integer<WithWidth>& a, const Integer<Other>& b) noexcept
        {
            union
            {
                WithWidth i;
                std::make_unsigned_t<WithWidth> u;
            } ret { .u = a.value };
            ret.u -= std::make_unsigned_t<WithWidth>(b.value);
            return ret.i;
        }
        template <std::integral Other>
        friend constexpr Integer<typename TypeLargestOf<WithWidth, Other>::Type> operator*(const Integer<WithWidth>& a, const Integer<Other>& b) noexcept
        {
            union
            {
                WithWidth i;
                std::make_unsigned_t<WithWidth> u;
            } ret { .u = a.value };
            ret.u *= std::make_unsigned_t<WithWidth>(b.value);
            return ret.i;
        }
        // todo: division, mod

        constexpr Integer operator~() const noexcept
        {
            return WithWidth(~std::make_unsigned_t<WithWidth>(this->value));
        }
        template <std::integral Other>
        friend constexpr Integer<typename TypeLargestOf<WithWidth, Other>::Type> operator&(const Integer<WithWidth>& a, const Integer<Other>& b) noexcept
        {
            return typename TypeLargestOf<WithWidth, Other>::Type(std::make_unsigned_t<typename TypeLargestOf<WithWidth, Other>::Type>(a.value) &
                                                                  std::make_unsigned_t<typename TypeLargestOf<WithWidth, Other>::Type>(b.value));
        }
        template <std::integral Other>
        friend constexpr Integer<typename TypeLargestOf<WithWidth, Other>::Type> operator|(const Integer<WithWidth>& a, const Integer<Other>& b) noexcept
        {
            return typename TypeLargestOf<WithWidth, Other>::Type(std::make_unsigned_t<typename TypeLargestOf<WithWidth, Other>::Type>(a.value) |
                                                                  std::make_unsigned_t<typename TypeLargestOf<WithWidth, Other>::Type>(b.value));
        }
        template <std::integral Other>
        friend constexpr Integer<typename TypeLargestOf<WithWidth, Other>::Type> operator^(const Integer<WithWidth>& a, const Integer<Other>& b) noexcept
        {
            return typename TypeLargestOf<WithWidth, Other>::Type(std::make_unsigned_t<typename TypeLargestOf<WithWidth, Other>::Type>(a.value) ^
                                                                  std::make_unsigned_t<typename TypeLargestOf<WithWidth, Other>::Type>(b.value));
        }
        template <std::unsigned_integral Other>
        friend constexpr Integer operator<<(const Integer<WithWidth>& a, const Integer<Other>& b) noexcept
        {
            return T(std::make_unsigned_t<WithWidth>(a.value) << b.value);
        }
        template <std::unsigned_integral Other>
        friend constexpr Integer operator>>(const Integer<WithWidth>& a, const Integer<Other>& b) noexcept
        {
            return T(std::make_unsigned_t<WithWidth>(a.value) >> b.value);
        }

        template <std::integral T>
        constexpr Integer& operator=(const Integer<T>& other) noexcept
        {
            *this = other;
            return *this;
        }
        template <std::integral T>
        constexpr Integer& operator+=(const Integer<T>& other) noexcept
        {
            *this = *this + other;
            return *this;
        }
        template <std::integral T>
        constexpr Integer& operator-=(const Integer<T>& other) noexcept
        {
            *this = *this - other;
            return *this;
        }
        template <std::integral T>
        constexpr Integer& operator*=(const Integer<T>& other) noexcept
        {
            *this = *this * other;
            return *this;
        }
        template <std::integral T>
        constexpr Integer& operator&=(const Integer<T>& other) noexcept
        {
            *this = *this & other;
            return *this;
        }
        template <std::integral T>
        constexpr Integer& operator|=(const Integer<T>& other) noexcept
        {
            *this = *this | other;
            return *this;
        }
        template <std::integral T>
        constexpr Integer& operator^=(const Integer<T>& other) noexcept
        {
            *this = *this ^ other;
            return *this;
        }
        template <std::unsigned_integral T>
        constexpr Integer& operator<<=(const Integer<T>& other) noexcept
        {
            *this = *this << other;
            return *this;
        }
        template <std::unsigned_integral T>
        constexpr Integer& operator>>=(const Integer<T>& other) noexcept
        {
            *this = *this >> other;
            return *this;
        }
        // todo: diveq, modeq
    private:
        WithWidth value;
    };
} // namespace sys

using byte = unsigned char;
using sbyte = signed char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;
using llong = long long;
using ullong = unsigned long long;

using i8 = ::sys::Integer<int_least8_t>;
using i16 = ::sys::Integer<int_least16_t>;
using i32 = ::sys::Integer<int_least32_t>;
using i64 = ::sys::Integer<int_least64_t>;

using u8 = ::sys::Integer<uint_least8_t>;
using u16 = ::sys::Integer<uint_least16_t>;
using u32 = ::sys::Integer<uint_least32_t>;
using u64 = ::sys::Integer<uint_least64_t>;

using sz = ::sys::Integer<size_t>;
using ssz = ::sys::Integer<ptrdiff_t>;

_push_nowarn(_clWarn_literal_suffix);
constexpr i8 operator""i8(ullong lit) noexcept
{
    return i8(i8::UnderlyingType(lit));
}
constexpr i16 operator""i16(ullong lit) noexcept
{
    return i16(i16::UnderlyingType(lit));
}
constexpr i32 operator""i32(ullong lit) noexcept
{
    return i32(i32::UnderlyingType(lit));
}
constexpr i64 operator""i64(ullong lit) noexcept
{
    return i64(i64::UnderlyingType(lit));
}
constexpr u8 operator""u8(ullong lit) noexcept
{
    return u8(u8::UnderlyingType(lit));
}
constexpr u16 operator""u16(ullong lit) noexcept
{
    return u16(u16::UnderlyingType(lit));
}
constexpr u32 operator""u32(ullong lit) noexcept
{
    return u32(u32::UnderlyingType(lit));
}
constexpr u64 operator""u64(ullong lit) noexcept
{
    return u64(u64::UnderlyingType(lit));
}
_pop_nowarn();

#if !__STDCPP_FLOAT32_T__
using f32 = std::float32_t;
#endif
#if !__STDCPP_FLOAT64_T__
using f64 = std::float64_t;
#endif
