#pragma once

/// @file

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace sys
{
    // Basic

    /// @ingroup sys
    /// @brief Named requirement for default initializable types.
    template <typename T>
    concept IDefaultConstructible = std::default_initializable<T>;
    /// @ingroup sys
    /// @brief Named requirement for nothrow default constructible types.
    template <typename T>
    concept INothrowDefaultConstructible = std::is_nothrow_default_constructible_v<T>;

    /// @ingroup sys
    /// @brief Named requirement for copy constructible types.
    template <typename T>
    concept ICopyConstructible = std::copy_constructible<T>;
    /// @ingroup sys
    /// @brief Named requirement for nothrow copy constructible types.
    template <typename T>
    concept INothrowCopyConstructible = std::is_nothrow_copy_constructible_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for move constructible types.
    template <typename T>
    concept IMoveConstructible = std::move_constructible<T>;
    /// @ingroup sys
    /// @brief Named requirement for nothrow move constructible types.
    template <typename T>
    concept INothrowMoveConstructible = std::is_nothrow_move_constructible_v<T>;

    /// @ingroup sys
    /// @brief Named requirement for copy assignable types.
    template <typename T>
    concept ICopyAssignable = std::is_copy_assignable_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for nothrow copy assignable types.
    template <typename T>
    concept INothrowCopyAssignable = std::is_nothrow_copy_assignable_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for move assignable types.
    template <typename T>
    concept IMoveAssignable = std::is_move_assignable_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for nothrow move assignable types.
    template <typename T>
    concept INothrowMoveAssignable = std::is_nothrow_move_assignable_v<T>;

    /// @ingroup sys
    /// @brief Named requirement for destructible types.
    template <typename T>
    concept IDestructible = std::is_destructible_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for nothrow destructible types.
    template <typename T>
    concept INothrowDestructible = std::is_nothrow_destructible_v<T>;

    // Type Properties

    /// @ingroup sys
    /// @brief Named requirement for scalar types.
    template <typename T>
    concept IScalar = std::is_scalar_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for C-struct equivalent (trivial + standard-layout) types.
    template <typename T>
    concept ICStructEquivalent = std::is_trivial_v<T> && std::is_standard_layout_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for trivially copyable types.
    template <typename T>
    concept ITriviallyCopyable = std::is_trivially_copyable_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for trivially moveable types.
    template <typename T>
    concept ITriviallyMoveable = std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for trivial types.
    template <typename T>
    concept ITrivial = std::is_trivial_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for standard layout types.
    template <typename T>
    concept IStandardLayout = std::is_standard_layout_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for implicit lifetime types.
    template <typename T>
    concept IImplicitLifetime = IScalar<std::remove_cvref_t<T>> ||
        (std::is_aggregate_v<std::remove_cvref_t<T>> && std::is_trivially_constructible_v<std::remove_cvref_t<T>> && std::is_trivially_destructible_v<std::remove_cvref_t<T>>) ||
        std::is_array_v<std::remove_cvref_t<T>>;

    // Library-Wide

    /// @ingroup sys
    /// @brief Named requirement for boolean testable types.
    template <typename T>
    concept IBooleanTestable = std::convertible_to<T, bool> && requires(T&& t) {
        { !std::forward<T>(t) } -> std::convertible_to<bool>;
    };
    /// @ingroup sys
    /// @brief Named requirement for equality comparable types.
    template <typename T>
    concept IEqualityComparable = std::equality_comparable<T>;
    /// @ingroup sys
    /// @brief Named requirement for nothrow equality comparable types.
    template <typename T>
    concept INothrowEqualityComparable = IEqualityComparable<T> && requires(T a, T b) {
        { a == b } noexcept;
        { a != b } noexcept;
    };
    /// @ingroup sys
    /// @brief Named requirement for swappable types.
    template <typename T>
    concept ISwappable = std::is_swappable_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for nothrow swappable types.
    template <typename T>
    concept INothrowSwappable = std::is_nothrow_swappable_v<T>;
    /// @ingroup sys
    /// @brief Named requirement for value swappable types.
    template <typename T>
    concept IValueSwappable = ISwappable<decltype(*std::declval<T>())>;
    /// @ingroup sys
    /// @brief Named requirement for nothrow value swappable types.
    template <typename T>
    concept INothrowValueSwappable = INothrowSwappable<decltype(*std::declval<T>())>;
    /// @ingroup sys
    /// @brief Named requirement for nullable pointer types.
    template <typename T>
    concept INullablePointer = INothrowDefaultConstructible<T> && INothrowCopyConstructible<T> && INothrowMoveConstructible<T> && INothrowCopyAssignable<T> &&
        INothrowSwappable<T> && INothrowDestructible<T> && requires(T p, std::nullptr_t np) {
            { T(np) } noexcept;
            { p = np } noexcept;
            { p == T() } noexcept -> IBooleanTestable;
            { p != T() } noexcept -> IBooleanTestable;
            { p == np } noexcept -> IBooleanTestable;
            { p != np } noexcept -> IBooleanTestable;
            { np == p } noexcept -> IBooleanTestable;
            { np != p } noexcept -> IBooleanTestable;
        };
    /// @ingroup sys
    /// @brief Named requirement for function object types.
    template <typename T, typename... Args>
    concept IFunctionObject = std::is_object_v<T> && std::invocable<T, Args...>;
    /// @ingroup sys
    /// @brief Named requirement for hash object types.
    template <typename T, typename Key>
    concept IHash = IFunctionObject<T> && ICopyConstructible<T> && IDestructible<T> && requires(T h, const Key k) {
        { h(k) } -> std::same_as<size_t>;
    };
    /// @ingroup sys
    /// @brief Named requirement for callable types.
    template <typename T, typename... Args>
    concept ICallable = std::invocable<T, Args...>;
    /// @ingroup sys
    /// @brief Named requirement for unary predicate types.
    template <typename T, typename Arg>
    concept IUnaryPredicate = std::predicate<T, Arg>;
    /// @ingroup sys
    /// @brief Named requirement for binary predicate types.
    template <typename T, typename Left, typename Right>
    concept IBinaryPredicate = std::predicate<T, Left, Right>;
    /// @ingroup sys
    /// @brief Named requirement for compare types.
    template <typename T, typename Left, typename Right>
    concept ICompare = std::relation<T, Left, Right> && requires(T comp, Left a, Right b) {
        { !comp(a, b) && !comp(b, a) } -> std::same_as<bool>;
    };
}; // namespace sys
