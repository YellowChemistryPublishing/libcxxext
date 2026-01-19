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
        static constexpr bool is_signature_of = std::is_invocable_r_v<std::remove_cvref_t<T>, std::remove_cvref_t<Func>, std::remove_cvref_t<Args>...>;

        function_signature() = delete;
    };

    template <typename T, typename... Args>
    struct function_signature<T(Args...)> : function_signature<T, Args...>
    { };

    template <typename... Pack>
    struct parameter_pack
    {
        using tuple = std::tuple<Pack...>;

        template <size_t Index>
        using at = std::tuple_element_t<Index, tuple>;

        template <typename T>
        static constexpr bool contains = (std::is_same_v<T, Pack> || ...);
    };
} // namespace sys::meta

namespace sys
{
    /// @brief Any number.
    template <typename T>
    concept IBuiltinNumeric = std::integral<T> || std::floating_point<T>;
    /// @brief Whether `T` can hold the entire range of `Than`.
    template <typename T, typename Than>
    concept IBuiltinIntegerCanHold = std::integral<T> && std::integral<Than> && std::cmp_less_equal(std::numeric_limits<T>::lowest(), std::numeric_limits<Than>::lowest()) &&
        std::cmp_greater_equal(std::numeric_limits<T>::max(), std::numeric_limits<Than>::max());

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
