#pragma once

#include <cstdio>          // NOLINT(misc-include-cleaner)
#include <print>           // NOLINT(misc-include-cleaner)
#include <source_location> // NOLINT(misc-include-cleaner)

#include <CompilerWarnings.h>
#include <Platform.h>

#define _catcat(a, b) a##b
#define _ppcat(a, b) _catcat(a, b)

#define _assert_ctor_can_fail() static_assert(false, "This constructor may fail, use `...::ctor` instead.")

/// @brief Tag type for function variants marked unsafe.
struct unsafe
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
/// @brief Mark a parameter (or this) as non-aliasing.
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

#if _libcxxext_compiler_msvc
#define _no_unique_address [[msvc::no_unique_address]]
#else
#define _no_unique_address [[no_unique_address]]
#endif

/// @defgroup Casts C++ Casts
/// @details Convenience macro aliases for C++ casts.

/// @addtogroup Casts
/// @{

/// @def _as
/// @brief Alias for `static_cast`.
#define _as(T, ...) static_cast<T>(__VA_ARGS__)
/// @def _asd
/// @brief Alias for `dynamic_cast`.
#define _asd(T, ...) dynamic_cast<T>(__VA_ARGS__)
/// @def _asc
/// @brief Alias for `const_cast`.
#define _asc(T, ...) const_cast<T>(__VA_ARGS__)
/// @def _asr
/// @brief Alias for `reinterpret_cast`.
#define _asr(T, ...) reinterpret_cast<T>(__VA_ARGS__)

/// @}

/// @def _throw(value)
/// @brief Logs a source location, and throws the value of the expression `value`.
#define _throw(value)                                                                                                                                                           \
    do                                                                                                                                                                          \
    {                                                                                                                                                                           \
        const std::source_location _src_loc = std::source_location::current();                                                                                                  \
        _push_nowarn_gcc(_clwarn_gcc_use_after_free);                                                                                                                           \
        _push_nowarn_clang(_clwarn_clang_use_after_free);                                                                                                                       \
        std::println(stderr, "In function `{}` at \"{}:{}:{}\" - Throwing `{}`.", _src_loc.function_name(), _src_loc.file_name(), int(_src_loc.line()), int(_src_loc.column()), \
                     typeid(decltype(value)).name());                                                                                                                           \
        _pop_nowarn_clang();                                                                                                                                                    \
        _pop_nowarn_gcc();                                                                                                                                                      \
        throw(value);                                                                                                                                                           \
    }                                                                                                                                                                           \
    while (false)

/// @defgroup EarlyReturnOperators Early Return Macros
/// @details Convenience macros for early return operations.

/// @addtogroup EarlyReturnOperators
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

/// @def _contract_assert(cond)
/// @brief Enforce a contract, throwing a `contract_violation_exception` if `cond` is false.
#define _contract_assert(cond)                                                                                             \
    do                                                                                                                     \
    {                                                                                                                      \
        const bool _expr = cond;                                                                                           \
        if (!_expr)                                                                                                        \
            _throw(::sys::contract_violation_exception("Contract violated, condition `" #cond "` evaluated to `false`.")); \
    }                                                                                                                      \
    while (false)
