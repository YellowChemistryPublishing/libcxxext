#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace sys
{
    // Basic

    template <typename T>
    concept IDefaultConstructible = std::default_initializable<T>;
    template <typename T>
    concept INothrowDefaultConstructible = std::is_nothrow_default_constructible_v<T>;

    template <typename T>
    concept ICopyConstructible = std::copy_constructible<T>;
    template <typename T>
    concept INothrowCopyConstructible = std::is_nothrow_copy_constructible_v<T>;
    template <typename T>
    concept IMoveConstructible = std::move_constructible<T>;
    template <typename T>
    concept INothrowMoveConstructible = std::is_nothrow_move_constructible_v<T>;

    template <typename T>
    concept ICopyAssignable = std::is_copy_assignable_v<T>;
    template <typename T>
    concept INothrowCopyAssignable = std::is_nothrow_copy_assignable_v<T>;
    template <typename T>
    concept IMoveAssignable = std::is_move_assignable_v<T>;
    template <typename T>
    concept INothrowMoveAssignable = std::is_nothrow_move_assignable_v<T>;

    template <typename T>
    concept IDestructible = std::is_destructible_v<T>;
    template <typename T>
    concept INothrowDestructible = std::is_nothrow_destructible_v<T>;

    // Type Properties

    template <typename T>
    concept IScalar = std::is_scalar_v<T>;
    template <typename T>
    concept ICStructEquivalent = std::is_trivial_v<T> && std::is_standard_layout_v<T>;
    template <typename T>
    concept ITriviallyCopyable = std::is_trivially_copyable_v<T>;
    template <typename T>
    concept ITriviallyMoveable = std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T>;
    template <typename T>
    concept ITrivial = std::is_trivial_v<T>;
    template <typename T>
    concept IStandardLayout = std::is_standard_layout_v<T>;
    template <typename T>
    concept IImplicitLifetime = IScalar<std::remove_cvref_t<T>> ||
        (std::is_aggregate_v<std::remove_cvref_t<T>> && std::is_trivially_constructible_v<std::remove_cvref_t<T>> && std::is_trivially_destructible_v<std::remove_cvref_t<T>>) ||
        std::is_array_v<std::remove_cvref_t<T>>;

    // Library-Wide

    template <typename T>
    concept IBooleanTestable = std::convertible_to<T, bool> && requires(T&& t) {
        { !std::forward<T>(t) } -> std::convertible_to<bool>;
    };
    template <typename T>
    concept IEqualityComparable = std::equality_comparable<T>;
    template <typename T>
    concept INothrowEqualityComparable = IEqualityComparable<T> && requires(T a, T b) {
        { a == b } noexcept;
        { a != b } noexcept;
    };
    template <typename T>
    concept ISwappable = std::is_swappable_v<T>;
    template <typename T>
    concept INothrowSwappable = std::is_nothrow_swappable_v<T>;
    template <typename T>
    concept IValueSwappable = ISwappable<decltype(*std::declval<T>())>;
    template <typename T>
    concept INothrowValueSwappable = INothrowSwappable<decltype(*std::declval<T>())>;
    template <typename T>
    concept INullablePointer = INothrowDefaultConstructible<T> && INothrowCopyConstructible<T> && INothrowMoveConstructible<T> && INothrowCopyAssignable<T> &&
        INothrowSwappable<T> && INothrowDestructible<T> && requires(T p, std::nullptr_t np) {
            { T(np) } noexcept;
            { p = np } noexcept;
            { p == T() } noexcept -> IBooleanTestable<>;
            { p != T() } noexcept -> IBooleanTestable<>;
            { p == np } noexcept -> IBooleanTestable<>;
            { p != np } noexcept -> IBooleanTestable<>;
            { np == p } noexcept -> IBooleanTestable<>;
            { np != p } noexcept -> IBooleanTestable<>;
        };
    template <typename T, typename... Args>
    concept IFunctionObject = std::is_object_v<T> && requires(T f, Args... args) { f(args...); };
    template <typename T, typename Key>
    concept IHash = IFunctionObject<T> && ICopyConstructible<T> && IDestructible<T> && requires(T h, const Key k) {
        { h(k) } -> std::same_as<size_t>;
    };
    template <typename T, typename... Args>
    concept ICallable = requires(T f) { std::invoke(f, std::declval<Args>()...); };
    template <typename T, typename Arg>
    concept IUnaryPredicate = IFunctionObject<T> && requires(T f, Arg arg) {
        { f(arg) } -> IBooleanTestable;
    };
    template <typename T, typename Left, typename Right>
    concept IBinaryPredicate = IFunctionObject<T> && requires(T f, Left a, Right b) {
        { f(a, b) } -> IBooleanTestable;
    };
    template <typename T, typename Left, typename Right>
    concept ICompare = IBinaryPredicate<T, Left, Right> && requires(T comp, Left a, Right b) {
        { !comp(a, b) && !comp(b, a) } -> std::same_as<bool>;
    };
}; // namespace sys
