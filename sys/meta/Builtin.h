#pragma once

/// @file

#include <concepts>
#include <limits>
#include <utility>

namespace sys
{
    // NOLINTBEGIN(google-runtime-int)

    /// @ingroup sys
    /// @brief Built-in signed integer type.
    template <typename T>
    concept IBuiltinIntegerSigned = std::signed_integral<T> && !std::same_as<std::remove_cv_t<T>, bool>;
    /// @ingroup sys
    /// @brief Built-in unsigned integer type.
    template <typename T>
    concept IBuiltinIntegerUnsigned = std::unsigned_integral<T> && !std::same_as<std::remove_cv_t<T>, bool>;

    // NOLINTEND(google-runtime-int)

    /// @ingroup sys
    /// @brief Built-in integer type.
    template <typename T>
    concept IBuiltinInteger = std::integral<T> && !std::same_as<std::remove_cv_t<T>, bool>;
    /// @ingroup sys
    /// @brief Built-in floating-point type.
    template <typename T>
    concept IBuiltinFloatingPoint = std::floating_point<T>;
    /// @ingroup sys
    /// @brief Built-in numeric type.
    template <typename T>
    concept IBuiltinNumeric = IBuiltinInteger<T> || IBuiltinFloatingPoint<T>;

    /// @ingroup sys
    /// @brief Whether `Wider` can hold the entire range of `Narrower`.
    template <typename Narrower, typename Wider>
    concept IBuiltinIntegerNarrowerThan =
        IBuiltinInteger<Wider> && IBuiltinInteger<Narrower> && std::cmp_less_equal(std::numeric_limits<Wider>::lowest(), std::numeric_limits<Narrower>::lowest()) &&
        std::cmp_greater_equal(std::numeric_limits<Wider>::max(), std::numeric_limits<Narrower>::max());

    /// @ingroup sys
    /// @brief Whether `T` represents a unicode character.
    template <typename T>
    concept IUnicodeCharacter = std::same_as<std::remove_cv_t<T>, char8_t> || std::same_as<std::remove_cv_t<T>, char16_t> || std::same_as<std::remove_cv_t<T>, char32_t>;
    /// @ingroup sys
    /// @brief Whether `T` is a character type.
    template <typename T>
    concept ICharacter = std::same_as<std::remove_cv_t<T>, char> || std::same_as<std::remove_cv_t<T>, wchar_t> || IUnicodeCharacter<std::remove_cv_t<T>>;
} // namespace sys
