#pragma once

/// @file

#include <cmath>
#include <limits>
#include <utility>

#include <LanguageSupport.h>
#include <Platform.h>
#include <Result.h>
#include <meta/Builtin.h>
#include <meta/Type.h>

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
    template <sys::IBuiltinInteger To>
    /* NOLINT(misc-use-internal-linkage) */ constexpr To bnumeric_cast(const sys::IBuiltinInteger auto value, decltype(unsafe)) noexcept
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
    template <sys::IBuiltinInteger To>
    /* NOLINT(misc-use-internal-linkage) */ constexpr To bnumeric_cast(const sys::IBuiltinFloatingPoint auto value, decltype(unsafe)) noexcept
    {
        if (std::isnan(value)) [[unlikely]] // Don't rearrange, MSVC will cook you on /fp:fast.
            return sys::bsentinel<To>();
        else if (value <= _as(std::numeric_limits<To>::lowest(), _decltype_of(value))) [[unlikely]]
            return std::numeric_limits<To>::lowest();
        else if (value >= _as(std::numeric_limits<To>::max(), _decltype_of(value))) [[unlikely]]
            return std::numeric_limits<To>::max();
        else [[likely]]
            return To(value);
    }

    /// @ingroup sys
    /// @brief Exact-value cast `From` to `To`, or error if the value is out of range.
    template <sys::IBuiltinInteger To>
    /* NOLINT(misc-use-internal-linkage) */ constexpr result<To> bnumeric_cast(const sys::IBuiltinInteger auto value) noexcept
    {
        if (std::cmp_less(value, std::numeric_limits<To>::lowest()) || std::cmp_greater(value, std::numeric_limits<To>::max())) [[unlikely]]
            return nullptr;
        else [[likely]]
            return To(value);
    }
    /// @ingroup sys
    /// @brief Truncating cast `From` to `To`, or error if the value is out of range.
    template <sys::IBuiltinInteger To>
    /* NOLINT(misc-use-internal-linkage) */ constexpr result<To> bnumeric_cast(const sys::IBuiltinFloatingPoint auto value) noexcept
    {
        // Don't invert condition, need to catch NaN.
        if (value >= _as(std::numeric_limits<To>::lowest(), _decltype_of(value)) && value <= _as(std::numeric_limits<To>::max(), _decltype_of(value))) [[likely]]
            return To(value);
        else [[unlikely]]
            return nullptr;
    }
} // namespace sys
