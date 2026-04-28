#pragma once

/// @file

#include <cassert>   // NOLINT(misc-include-cleaner)
#include <cstdint>   // NOLINT(misc-include-cleaner)
#include <cstdio>    // NOLINT(misc-include-cleaner)
#include <exception> // NOLINT(misc-include-cleaner)
#include <print>     // NOLINT(misc-include-cleaner)

#include <Platform.h>

/// @defgroup lib Libraries
/// @brief `libcxxext` library components.

/// @defgroup tags Categories
/// @brief Groups of related, often convenient, features.

/// @defgroup sys_internal sys[Internal]
/// @ingroup lib
/// @brief Internal implementation detail.

/// @defgroup sys sys
/// @ingroup lib
/// @brief Core system functionality.

/// @defgroup sys_containers sys.Containers
/// @ingroup lib
/// @brief Container library.

/// @defgroup sys_text sys.Text
/// @ingroup lib
/// @brief Unicode text handling and manipulation library.

/// @defgroup sys_threading sys.Threading
/// @ingroup lib
/// @brief Threading primitives and utilities.

/// @namespace sys
/// @brief Contains every public facing system API.

/// @namespace sys::internal
/// @brief Internal implementation detail.

/// @namespace sys::meta
/// @brief Type trait metadata support.

/// @def _xstr(x)
/// @ingroup sys
/// @brief Stringifies a token; used to avoid macro expansion.
#define _xstr(...) #__VA_ARGS__
/// @def _ppstr(x)
/// @ingroup sys
/// @brief Stringifies a token.
#define _ppstr(...) _xstr(__VA_ARGS__)

/// @def _catcat(a, b)
/// @ingroup sys
/// @brief Concatenates two tokens; used to avoid macro expansion.
#define _catcat(a, b) a##b
/// @def _ppcat(a, b)
/// @ingroup sys
/// @brief Concatenates two tokens.
#define _ppcat(a, b) _catcat(a, b)

/// @ingroup sys
/// @brief Tag value for function variants that are `unsafe`.
constexpr struct
{
} unsafe;

/// @def _inline_always
/// @ingroup sys
/// @brief Force inline a function.
#if !_libcxxext_compiler_msvc
#define _inline_always [[gnu::always_inline]] inline
#else
#define _inline_always __forceinline
#endif

/// @def _inline_never
/// @ingroup sys
/// @brief Force noinline a function.
#if !_libcxxext_compiler_msvc
#define _inline_never [[gnu::noinline]]
#else
#define _inline_never [[msvc::noinline]]
#endif

/// @def _weak
/// @ingroup sys
/// @brief Mark a function as weak.
#define _weak [[gnu::weak]]

/// @def _pure
/// @ingroup sys
/// @brief Mark a function as pure.
#define _pure [[gnu::pure]]

/// @def _pure_const
/// @ingroup sys
/// @brief Mark a function as pure and const.
#if !_libcxxext_compiler_msvc
#define _pure_const [[gnu::const]]
#else
#define _pure_const
#endif

/// @def _restrict
/// @ingroup sys
/// @brief Mark a parameter (or `this`) as non-aliasing.
#if _libcxxext_compiler_msvc
#define _restrict __restrict
#else
#define _restrict __restrict__
#endif

/// @def _pack(align)
/// @ingroup sys
/// @brief Pack a structure to `align` bytes.
#define _pack(align) _clpragma_fwd(pack(align))

/// @def _packed
/// @ingroup sys
/// @brief Pack a structure to the smallest possible alignment.
#if !_libcxxext_compiler_msvc
#define _packed [[gnu::packed]]
#else
#define _packed __declspec(align(1))
#endif

/// @def _no_unique_address
/// @ingroup sys
/// @brief Mark a member variable as possibly zero-size.
#if _libcxxext_compiler_msvc
#define _no_unique_address [[msvc::no_unique_address]]
#else
#define _no_unique_address [[no_unique_address]]
#endif

/// @defgroup casts C++ Casts
/// @ingroup tags
/// @brief Convenience macro aliases for C++ casts.
/// @{

/// @def _as(expr, ...)
/// @ingroup sys
/// @brief Alias for `static_cast`.
#define _as(expr, ...) static_cast<__VA_ARGS__>(expr)
/// @def _asd(expr, ...)
/// @ingroup sys
/// @brief Alias for `dynamic_cast`.
#define _asd(expr, ...) dynamic_cast<__VA_ARGS__>(expr)
/// @def _asc(expr, ...)
/// @ingroup sys
/// @brief Alias for `const_cast`.
#define _asc(expr, ...) const_cast<__VA_ARGS__>(expr)
/// @def _asr(expr, ...)
/// @ingroup sys
/// @brief Alias for `reinterpret_cast`.
#define _asr(expr, ...) reinterpret_cast<__VA_ARGS__>(expr)

/// @}

/// @defgroup builtin_integers Built-in Integer Types
/// @ingroup tags
/// @ingroup sys
/// @brief Abbreviated built-in integer types.
/// @note Pass `byval`.
/// @{

// NOLINTBEGIN(google-runtime-int)
/// @brief Alias for `unsigned char`.
using byte = unsigned char;
/// @brief Alias for `signed char`.
using sbyte = signed char;
/// @brief Alias for `unsigned short`.
using ushort = unsigned short;
/// @brief Alias for `unsigned int`.
using uint = unsigned int;
/// @brief Alias for `unsigned long`.
using ulong = unsigned long;
/// @brief Alias for `long long`.
using llong = long long;
/// @brief Alias for `unsigned long long`.
using ullong = unsigned long long;
// NOLINTEND(google-runtime-int)

/// @}

#ifndef _contract_assert
/// @def _contract_assert(cond, ...)
/// @ingroup sys
/// @brief Enforce a contract, asserting if `cond` is `false`.
#define _contract_assert(cond, ...)                                                                                                      \
    do                                                                                                                                   \
    {                                                                                                                                    \
        if (!(cond)) /* NOLINT(readability-simplify-boolean-expr) */                                                                     \
        {                                                                                                                                \
            try                                                                                                                          \
            {                                                                                                                            \
                std::println(stderr, "Contract violated, condition `" #cond "` evaluated to `false`." __VA_OPT__(" (" __VA_ARGS__ ")")); \
            }                                                                                                                            \
            catch (...)                                                                                                                  \
            { }                                                                                                                          \
                                                                                                                                         \
            volatile uint_least64_t i = 5'000'000'000;                                                                                   \
            while (i > 0)                                                                                                                \
                i = i - 1;                                                                                                               \
                                                                                                                                         \
            std::terminate();                                                                                                            \
        }                                                                                                                                \
    }                                                                                                                                    \
    while (false)
#endif

/// @defgroup early_return_operators Early Return Macros
/// @ingroup tags
/// @brief Convenience macros for early return operations.
/// @{

/// @def _retif(val, cond)
/// @ingroup sys
/// @brief Return `val` if `cond` is `true`.
/// @note Returning must not be the happy path.
#define _retif(val, cond)      \
    do                         \
    {                          \
        if (cond) [[unlikely]] \
            return val;        \
    }                          \
    while (false)
/// @def _coretif(val, cond)
/// @ingroup sys
/// @brief Coroutine-return `val` if `cond` is `true`.
/// @note Returning must not be the happy path.
#define _coretif(val, cond)                                         \
    do                                                              \
    {                                                               \
        if (cond) [[unlikely]]                                      \
            co_return val /* NOLINT(bugprone-macro-parentheses) */; \
    }                                                               \
    while (false)

/// @}
