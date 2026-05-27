#pragma once

/// @file

#include <concepts>
#include <cstddef>
#include <iterator>
#include <utility>

#include <LanguageSupport.h>
#include <meta/NamedRequirements.h>
#include <meta/Type.h>

namespace sys::internal
{
    /// @internal
    /// @ingroup sys_internal
    template <typename T, typename... Args>
    concept IEmpty = requires(T range) {
        { range.empty() } -> IBooleanTestable;
    };
    /// @internal
    /// @ingroup sys_internal
    template <typename T, typename... Args>
    concept IStdSize = requires(T range) {
        { std::size(range) } -> std::same_as<size_t>;
    };

    template <typename T, typename... Args>
    concept IEmplaceBack = requires(T range, Args&&... args) { range.emplace_back(_forward(args)...); };
    /// @internal
    /// @ingroup sys_internal
    template <typename T, typename... Args>
    concept IPushBack = requires(T range, Args&&... args) { range.push_back(_forward(args)...); };
    /// @internal
    /// @ingroup sys_internal
    template <typename T, typename... Args>
    concept IEmplace = requires(T range, Args&&... args) { range.emplace(_forward(args)...); };
    /// @internal
    /// @ingroup sys_internal
    template <typename T, typename... Args>
    concept IInsert = requires(T range, Args&&... args) { range.insert(_forward(args)...); };
    /// @internal
    /// @ingroup sys_internal
    template <typename T, typename... Args>
    concept IPush = requires(T range, Args&&... args) { range.push(_forward(args)...); };
    /// @internal
    /// @ingroup sys_internal
    template <typename T, typename... Args>
    concept IAppend = requires(T range, Args&&... args) { range.append(_forward(args)...); };
    /// @internal
    /// @ingroup sys_internal
    template <typename T, typename... Args>
    concept IStreamable = requires(T range, Args&&... args) { (range << ... << _forward(args)); };
} // namespace sys::internal

namespace sys::meta
{
    /// @ingroup sys
    template <typename T>
    requires (!type<T>::is_ref())
    struct generic_container_adaptor
    {
    private:
        T& range; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    public:
        constexpr /* NOLINT(hicpp-explicit-conversions) */ generic_container_adaptor(T& range) noexcept : range(range) { }

        /// @brief Check whether an empty-queryable `this->range` is empty.
        [[nodiscard]] constexpr bool empty(this auto&& _this)
        requires internal::IEmpty<T&> || internal::IStdSize<T&>
        {
            if constexpr (internal::IEmpty<T&>)
                return _this.range.empty();
            else if constexpr (internal::IStdSize<T&>)
                return std::size(_this.range) == _as(0, decltype(std::size(_this.range)));
        }
        [[nodiscard]] constexpr bool empty(this auto&&) = delete;

        /// @brief (Potentially) inplace construct and append an element to an appendable `this->range`.
        template <typename... Args>
        constexpr void append_back(Args&&... args)
        requires internal::IEmplaceBack<T&, Args...> || internal::IPushBack<T&, Args...> || internal::IPush<T&, Args...> || internal::IEmplace<T&, Args...> ||
            internal::IInsert<T&, Args...> || internal::IAppend<T&, Args...> || (internal::IStreamable<T&, Args...> && sizeof...(Args) > 0)
        {
            if constexpr (internal::IEmplaceBack<T&, Args...>)
                this->range.emplace_back(_forward(args)...);
            else if constexpr (internal::IPushBack<T&, Args...>)
                this->range.push_back(_forward(args)...);
            else if constexpr (internal::IEmplace<T&, Args...>)
                this->range.emplace(_forward(args)...); // LCOV_EXCL_BR_LINE: I actually have no idea why, but I promise you this will work.
            else if constexpr (internal::IInsert<T&, Args...>)
                this->range.insert(_forward(args)...);
            else if constexpr (internal::IPush<T&, Args...>)
                this->range.push(_forward(args)...);
            else if constexpr (internal::IAppend<T&, Args...>)
                this->range.append(_forward(args)...);
            else if constexpr (internal::IStreamable<T&, Args...> && sizeof...(Args) > 0)
                (this->range << ... << _forward(args));
        }
        constexpr void append_back(this auto&&) = delete;
    };

    template <typename T>
    generic_container_adaptor(const T&) -> generic_container_adaptor<const T>;
    template <typename T>
    generic_container_adaptor(T&) -> generic_container_adaptor<T>;
    template <typename T, size_t N>
    generic_container_adaptor(const T (&)[N]) -> generic_container_adaptor<const T[N]>;
    template <typename T, size_t N>
    generic_container_adaptor(T (&)[N]) -> generic_container_adaptor<T[N]>;
} // namespace sys::meta

namespace sys
{
    /// @ingroup sys
    /// @brief Whether `T` is sizeable.
    template <typename T, typename U = size_t>
    concept ISizeable = requires(T& range) {
        requires !std::same_as<U, void>;
        { std::size(range) } -> std::same_as<U>;
    } || requires(T& range) {
        requires std::same_as<U, void>;
        std::size(range);
    };

    /// @ingroup sys
    /// @brief Whether `T` is iterable.
    template <typename T, typename U = void>
    concept IEnumerable = requires(T& range, _decltype_of(std::begin(range)) it) {
        std::begin(range);
        std::end(range);

        std::begin(range) != std::end(range);
        ++it;

        requires requires {
            requires std::same_as<U, void>;
            *std::begin(range);
        } || requires {
            requires !std::same_as<U, void>;
            requires std::same_as<_decltype_of(*std::begin(range)), U>;
        };
    };

    /// @ingroup sys
    /// @brief Whether `T` can be checked for emptiness.
    template <typename T>
    concept IEmptyQueryable = requires(T& range) {
        { meta::generic_container_adaptor(range).empty() } -> IBooleanTestable;
    };
    /// @ingroup sys
    /// @brief Whether `T` can be appended to.
    template <typename T, typename... U>
    concept IAppendable = requires(T& range) { meta::generic_container_adaptor(range).append_back(std::declval<U&&>()...); };

    template <typename T>
    concept IContiguousRange = requires(T& range) {
        { std::data(range) } -> std::same_as<std::remove_pointer_t<decltype(std::data(range))>*>;
        requires requires {
            { std::size(range) } -> std::convertible_to<ptrdiff_t>;
        } || requires {
            { std::size(range) } -> std::convertible_to<size_t>;
        };
    };
} // namespace sys
