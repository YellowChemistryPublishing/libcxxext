#pragma once

/// @file

#include <concepts>
#include <limits>
#include <utility>

namespace sys
{
    // NOLINTBEGIN(google-runtime-int)

    /// @brief Built-in signed integer type.
    template <typename T>
    concept IBuiltinIntegerSigned =
        std::same_as<T, signed char> || std::same_as<T, signed short> || std::same_as<T, signed int> || std::same_as<T, signed long> || std::same_as<T, signed long long>;
    /// @brief Built-in unsigned integer type.
    template <typename T>
    concept IBuiltinIntegerUnsigned =
        std::same_as<T, unsigned char> || std::same_as<T, unsigned short> || std::same_as<T, unsigned int> || std::same_as<T, unsigned long> || std::same_as<T, unsigned long long>;

    // NOLINTEND(google-runtime-int)

    /// @brief Built-in integer type.
    template <typename T>
    concept IBuiltinInteger = IBuiltinIntegerSigned<T> || IBuiltinIntegerUnsigned<T>;
    /// @brief Built-in floating-point type.
    template <typename T>
    concept IBuiltinFloatingPoint = std::same_as<T, float> || std::same_as<T, double> || std::same_as<T, long double>;
    /// @brief Built-in numeric type.
    template <typename T>
    concept IBuiltinNumeric = IBuiltinInteger<T> || IBuiltinFloatingPoint<T>;

    /// @brief Whether `T` can hold the entire range of `CanHold`.
    template <typename T, typename CanHold>
    concept IBuiltinIntegerCanHold =
        IBuiltinInteger<T> && IBuiltinInteger<CanHold> && std::cmp_less_equal(std::numeric_limits<T>::lowest(), std::numeric_limits<CanHold>::lowest()) &&
        std::cmp_greater_equal(std::numeric_limits<T>::max(), std::numeric_limits<CanHold>::max());

    /// @brief Whether `T` represents a unicode character.
    template <typename T>
    concept IUnicodeCharacter = std::same_as<T, char8_t> || std::same_as<T, char16_t> || std::same_as<T, char32_t>;
    /// @brief Whether `T` is a character type.
    template <typename T>
    concept ICharacter = std::same_as<T, char> || std::same_as<T, wchar_t> || IUnicodeCharacter<T>;

} // namespace sys
