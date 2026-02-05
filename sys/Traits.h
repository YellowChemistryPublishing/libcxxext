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
    template <typename MetaMeta, typename T = void, typename... Args>
    struct function_signature
    {
        using return_type = T;
        using arguments = std::tuple<Args...>;

        template <typename = void>
        static consteval bool is_valid()
        {
            if constexpr (requires {
                              { MetaMeta::override_is_valid() } -> std::convertible_to<bool>;
                          })
                return MetaMeta::override_is_valid();
            else
                return false;
        }

        template <typename = void>
        static consteval bool is_member_func()
        {
            if constexpr (requires {
                              { MetaMeta::override_is_member_func() } -> std::convertible_to<bool>;
                          })
                return MetaMeta::override_is_member_func();
            else
                return false;
        }

        template <typename Func>
        static consteval bool is_signature_of()
        {
            if constexpr (requires {
                              { MetaMeta::template override_is_signature_of<Func>() } -> std::convertible_to<bool>;
                          })
                return MetaMeta::template override_is_signature_of<Func>();
            else if constexpr (function_signature::is_valid())
                return std::is_invocable_r_v<T, std::remove_cvref_t<Func>, Args...>;
            else
                return false;
        }

        function_signature() = delete;
    };

    template <typename T, typename... Args>
    struct function_signature<T(Args...)> : function_signature<function_signature<T(Args...)>, T, Args...>
    {
        static consteval bool override_is_valid() { return true; }
    };
    template <typename T, typename... Args>
    struct function_signature<T (*)(Args...)> : function_signature<function_signature<T (*)(Args...)>, T, Args...>
    {
        static consteval bool override_is_valid() { return true; }
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...)> : function_signature<function_signature<T (For::*)(Args...)>, T, Args...>
    {
        static consteval bool override_is_valid() { return true; }

        static consteval bool override_is_member_func() { return true; }
    };

    template <typename... Pack>
    struct parameter_pack final
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
    concept IFunc = meta::function_signature<Signature>::template is_signature_of<Functor>();

    /// @brief Whether `T` is iterable.
    template <typename T, typename U = void>
    concept IEnumerable = requires(T& range, std::remove_cvref_t<decltype(range.begin())> it) {
        range.begin();
        range.end();

        range.begin() != range.end();
        ++it;

        requires ((std::same_as<U, void> && requires { *range.begin(); }) || std::same_as<std::remove_cvref_t<decltype(*range.begin())>, U>);
    } || requires(T& range, std::remove_cvref_t<decltype(std::begin(range))> it) {
        std::begin(range);
        std::end(range);

        std::begin(range) != std::end(range);
        ++it;

        requires ((std::same_as<U, void> && requires { *std::begin(range); }) || std::same_as<std::remove_cvref_t<decltype(*std::begin(range))>, U>);
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
