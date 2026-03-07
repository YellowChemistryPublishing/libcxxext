#pragma once

/// @file

#include <cassert>         // NOLINT(misc-include-cleaner)
#include <cstdint>         // NOLINT(misc-include-cleaner)
#include <cstdio>          // NOLINT(misc-include-cleaner)
#include <exception>       // NOLINT(misc-include-cleaner)
#include <print>           // NOLINT(misc-include-cleaner)
#include <source_location> // NOLINT(misc-include-cleaner)

#include <Platform.h>

/// @namespace sys
/// @brief Contains every public facing system API.

/// @namespace sys::internal
/// @brief Internal implementation detail.

/// @namespace sys::meta
/// @brief Type trait metadata support.

/// @def _xstr(x)
/// @brief Stringifies a token; used to avoid macro expansion.
#define _xstr(...) #__VA_ARGS__
/// @def _ppstr(x)
/// @brief Stringifies a token.
#define _ppstr(...) _xstr(__VA_ARGS__)

/// @def _catcat(a, b)
/// @brief Concatenates two tokens; used to avoid macro expansion.
#define _catcat(a, b) a##b
/// @def _ppcat(a, b)
/// @brief Concatenates two tokens.
#define _ppcat(a, b) _catcat(a, b)

/// @def _assert_ctor_can_fail()
/// @brief Reminder to use `...::%ctor` instead of a constructor when the constructor may fail.
#define _assert_ctor_can_fail() static_assert(false, "This constructor may fail, use `...::ctor` instead.")

/// @brief Tag type for function variants marked `unsafe`.
struct unsafe final
{ };

/// @def _inline_always
/// @brief Force inline a function.
#if !_libcxxext_compiler_msvc
#define _inline_always [[gnu::always_inline]] inline
#else
#define _inline_always __forceinline
#endif

/// @def _inline_never
/// @brief Force noinline a function.
#if !_libcxxext_compiler_msvc
#define _inline_never [[gnu::noinline]]
#else
#define _inline_never [[msvc::noinline]]
#endif

/// @def _weak
/// @brief Mark a function as weak.
#define _weak [[gnu::weak]]

/// @def _pure
/// @brief Mark a function as pure.
#define _pure [[gnu::pure]]

/// @def _pure_const
/// @brief Mark a function as pure and const.
#if !_libcxxext_compiler_msvc
#define _pure_const [[gnu::const]]
#else
#define _pure_const
#endif

/// @def _restrict
/// @brief Mark a parameter (or `this`) as non-aliasing.
#define _restrict __restrict__

/// @def _pack(align)
/// @brief Pack a structure to `align` bytes.
#define _pack(align) _clpragma_fwd(pack(align))

/// @def _packed
/// @brief Pack a structure to the smallest possible alignment.
#if !_libcxxext_compiler_msvc
#define _packed [[gnu::packed]]
#else
#define _packed __declspec(align(1))
#endif

/// @def _no_unique_address
/// @brief Mark a member variable as possibly zero-size.
#if _libcxxext_compiler_msvc
#define _no_unique_address [[msvc::no_unique_address]]
#else
#define _no_unique_address [[no_unique_address]]
#endif

/// @defgroup casts C++ Casts
/// @brief Convenience macro aliases for C++ casts.
/// @{

/// @def _as(T, ...)
/// @brief Alias for `static_cast`.
#define _as(T, ...) static_cast<T>(__VA_ARGS__)
/// @def _asd(T, ...)
/// @brief Alias for `dynamic_cast`.
#define _asd(T, ...) dynamic_cast<T>(__VA_ARGS__)
/// @def _asc(T, ...)
/// @brief Alias for `const_cast`.
#define _asc(T, ...) const_cast<T>(__VA_ARGS__)
/// @def _asr(T, ...)
/// @brief Alias for `reinterpret_cast`.
#define _asr(T, ...) reinterpret_cast<T>(__VA_ARGS__)

/// @}

#ifndef _contract_assert
/// @def _contract_assert(cond, ...)
/// @brief Enforce a contract, asserting if `cond` is `false`.
#define _contract_assert(cond, ...)                                                                                                  \
    do                                                                                                                               \
    {                                                                                                                                \
        if (!(cond))                                                                                                                 \
        {                                                                                                                            \
            std::println(stderr, "Contract violated, condition `" #cond "` evaluated to `false`." __VA_OPT__(" (" __VA_ARGS__ ")")); \
            volatile uint_least64_t i = 5'000'000'000;                                                                               \
            while (i--)                                                                                                              \
            { }                                                                                                                      \
            std::terminate();                                                                                                        \
        }                                                                                                                            \
    }                                                                                                                                \
    while (false)
#endif

/// @defgroup early_return_operators Early Return Macros
/// @brief Convenience macros for early return operations.
/// @{

/// @def _retif(val, cond)
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
