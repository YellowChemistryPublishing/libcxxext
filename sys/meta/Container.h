#pragma once

/// @file

#include <concepts>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

#include <LanguageSupport.h>

namespace sys::meta
{
    template <typename T>
    struct generic_container_adaptor
    {
    private:
        T& range; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    public:
        constexpr /* NOLINT(hicpp-explicit-conversions) */ generic_container_adaptor(T& range) noexcept : range(range) { }

        /// @brief Check whether an empty-queryable `this->range` is empty.
        [[nodiscard]] constexpr bool empty() const
        {
            if constexpr (requires { this->range.empty(); })
                return this->range.empty();
            else if constexpr (requires { std::size(this->range); })
                return std::size(this->range) == _as(decltype(std::size(this->range)), 0);
            else if constexpr (requires { requires false; })
            { }
        }
        /// @brief (Potentially) inplace construct and append an element to an appendable `this->range`.
        template <typename... Args>
        constexpr void append_back(Args&&... args)
        {
            if constexpr (requires { this->range.emplace_back(std::forward<Args>(args)...); })
                this->range.emplace_back(std::forward<Args>(args)...);
            else if constexpr (requires { this->range.push_back(std::forward<Args>(args)...); })
                this->range.push_back(std::forward<Args>(args)...);
            else if constexpr (requires { this->range.push(std::forward<Args>(args)...); })
                this->range.push(std::forward<Args>(args)...);
            else if constexpr (requires { this->range.append(std::forward<Args>(args)...); })
                this->range.append(std::forward<Args>(args)...);
            else if constexpr (requires { (this->range << ... << std::forward<Args>(args)); })
                (this->range << ... << std::forward<Args>(args));
            else if constexpr (requires { requires false; })
            { }
        }
    };
} // namespace sys::meta

namespace sys
{
    /// @brief Whether `T` is sizeable.
    template <typename T, typename U = size_t>
    concept ISizeable = requires(T range) {
        requires !std::same_as<U, void>;
        { std::size(range) } -> std::same_as<U>;
    } || requires(T range) {
        requires std::same_as<U, void>;
        std::size(range);
    };

    /// @brief Whether `T` is iterable.
    template <typename T, typename U = void>
    concept IEnumerable = requires(T& range, std::remove_cvref_t<decltype(std::begin(range))> it) {
        std::begin(range);
        std::end(range);

        std::begin(range) != std::end(range);
        ++it;

        requires (requires {
            requires std::same_as<U, void>;
            *std::begin(range);
        } || std::same_as<std::remove_cvref_t<decltype(*std::begin(range))>, U>);
    };

    /// @brief Whether `T` can be checked for emptiness.
    template <typename T>
    concept IEmptyQueryable = requires(T range) { meta::generic_container_adaptor(range).empty(); };
    /// @brief Whether `T` can be appended to.
    template <typename T, typename... U>
    concept IAppendable = requires(T range) {
        requires (sizeof...(U) > 0);
        meta::generic_container_adaptor(range).append_back(std::declval<U>()...);
    };
} // namespace sys
