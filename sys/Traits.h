#pragma once

#include <concepts>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace sys::meta
{
    template <typename T, typename... Args>
    struct function_signature
    {
        using return_type = T;
        using arguments = std::tuple<Args...>;

        template <typename Func>
        static consteval bool is_signature_of()
        {
            return std::is_invocable_r_v<T, std::remove_cvref_t<Func>, Args...>;
        }

        function_signature() = delete;
    };

    template <typename T, typename... Args>
    struct function_signature<T(Args...)> : function_signature<T, Args...>
    {
        static consteval bool is_member_func() { return false; }
    };
    template <typename T, typename... Args>
    struct function_signature<T (*)(Args...)> : function_signature<T, Args...>
    {
        static consteval bool is_member_func() { return false; }
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...)> : function_signature<T, Args...>
    {
        static consteval bool is_member_func() { return true; }
    };

    template <typename... Pack>
    struct parameter_pack
    {
        using tuple = std::tuple<Pack...>;

        template <size_t Index>
        using at = std::tuple_element_t<Index, tuple>;

        template <typename T>
        static consteval bool contains()
        {
            return (std::is_same_v<T, Pack> || ...);
        }

        parameter_pack() = delete;
    };
} // namespace sys::meta

namespace sys
{
    // NOLINTBEGIN(google-runtime-int)

    // @brief Built-in signed integer type.
    template <typename T>
    concept IBuiltinIntegerSigned =
        std::same_as<T, signed char> || std::same_as<T, signed short> || std::same_as<T, signed int> || std::same_as<T, signed long> || std::same_as<T, signed long long>;
    // @brief Built-in unsigned integer type.
    template <typename T>
    concept IBuiltinIntegerUnsigned =
        std::same_as<T, unsigned char> || std::same_as<T, unsigned short> || std::same_as<T, unsigned int> || std::same_as<T, unsigned long> || std::same_as<T, unsigned long long>;

    // NOLINTEND(google-runtime-int)

    // @brief Built-in integer type.
    template <typename T>
    concept IBuiltinInteger = IBuiltinIntegerSigned<T> || IBuiltinIntegerUnsigned<T>;
    // @brief Built-in floating-point type.
    template <typename T>
    concept IBuiltinFloatingPoint = std::same_as<T, float> || std::same_as<T, double> || std::same_as<T, long double>;
    // @brief Built-in numeric type.
    template <typename T>
    concept IBuiltinNumeric = IBuiltinInteger<T> || IBuiltinFloatingPoint<T>;

    /// @brief Whether `T` can hold the entire range of `CanHold`.
    template <typename T, typename CanHold>
    concept IBuiltinIntegerCanHold =
        IBuiltinInteger<T> && IBuiltinInteger<CanHold> && std::cmp_less_equal(std::numeric_limits<T>::lowest(), std::numeric_limits<CanHold>::lowest()) &&
        std::cmp_greater_equal(std::numeric_limits<T>::max(), std::numeric_limits<CanHold>::max());

    /// @brief Whether `T` is a character type.
    template <typename T>
    concept ICharacter = std::same_as<T, char> || std::same_as<T, wchar_t> || std::same_as<T, char8_t> || std::same_as<T, char16_t> || std::same_as<T, char32_t>;

    /// @brief Whether `Functor` represents a function with signature `Signature`.
    template <typename Functor, typename Signature> // i.e. void(int, int)
    concept IFunc = meta::function_signature<Signature>::template is_signature_of<Functor>;

    /// @brief Whether `T` is iterable.
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

    /// @brief Whether `T` is sizeable.
    template <typename T, typename U = void>
    concept ISizeable = requires(T range) {
        requires (!std::same_as<U, void> && (requires {
                     { range.size() } -> std::same_as<U>;
                 } || requires {
                     { std::size(range) } -> std::same_as<U>;
                 })) || (std::same_as<U, void> && (requires { range.size(); } || requires { std::size(range); }));
    };
} // namespace sys
