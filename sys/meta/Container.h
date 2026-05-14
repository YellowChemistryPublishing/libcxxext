#pragma once

/// @file

#include <concepts>
#include <cstddef>
#include <iterator>
#include <utility>

#include <LanguageSupport.h>
#include <meta/Type.h>

namespace sys::meta
{
    template <typename...>
    inline constexpr bool dependent_false = false;

    /// @ingroup sys
    template <typename T>
    struct generic_container_adaptor
    {
    private:
        T& range; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    public:
        constexpr /* NOLINT(hicpp-explicit-conversions) */ generic_container_adaptor(T& range) noexcept : range(range) { }

        /// @brief Check whether an empty-queryable `this->range` is empty.
        [[nodiscard]] constexpr bool empty(this auto&& _this)
        requires (requires { _this.range.empty(); } || requires { std::size(_this.range); })
        {
            if constexpr (requires { _this.range.empty(); })
                return _this.range.empty();
            else if constexpr (requires { std::size(_this.range); })
                return std::size(_this.range) == _as(0, decltype(std::size(_this.range)));
        }
        [[nodiscard]] constexpr bool empty(this auto&&) = delete;
        /// @brief (Potentially) inplace construct and append an element to an appendable `this->range`.
        template <typename... Args>
        constexpr void append_back(Args&&... args)
        requires (requires { this->range.emplace_back(_forward(args)...); } || requires { this->range.push_back(_forward(args)...); } ||
                  requires { this->range.push(_forward(args)...); } || requires { this->range.append(_forward(args)...); } || requires {
                      requires sizeof...(Args) > 0;
                      (this->range << ... << _forward(args));
                  })
        {
            if constexpr (requires { this->range.emplace_back(_forward(args)...); })
                this->range.emplace_back(_forward(args)...);
            else if constexpr (requires { this->range.push_back(_forward(args)...); })
                this->range.push_back(_forward(args)...);
            else if constexpr (requires { this->range.push(_forward(args)...); })
                this->range.push(_forward(args)...);
            else if constexpr (requires { this->range.append(_forward(args)...); })
                this->range.append(_forward(args)...);
            else if constexpr (requires {
                                   requires sizeof...(Args) > 0;
                                   (this->range << ... << _forward(args));
                               })
                (this->range << ... << _forward(args));
        }
        constexpr void append_back(this auto&&) = delete;
    };
} // namespace sys::meta

namespace sys
{
    /// @ingroup sys
    /// @brief Whether `T` is sizeable.
    template <typename T, typename U = size_t>
    concept ISizeable = requires(T range) {
        requires !std::same_as<U, void>;
        { std::size(range) } -> std::same_as<U>;
    } || requires(T range) {
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

        requires (requires {
            requires std::same_as<U, void>;
            *std::begin(range);
        } || std::same_as<_decltype_of(*std::begin(range)), U>);
    };

    /// @ingroup sys
    /// @brief Whether `T` can be checked for emptiness.
    template <typename T>
    concept IEmptyQueryable = requires(T range) { meta::generic_container_adaptor(range).empty(); };
    /// @ingroup sys
    /// @brief Whether `T` can be appended to.
    template <typename T, typename... U>
    concept IAppendable = requires(T range) {
        { meta::generic_container_adaptor(range).append_back(std::declval<U>()...) } -> std::same_as<void>;
    };
} // namespace sys
