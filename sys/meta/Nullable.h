#pragma once

/// @file

#include <concepts>

#include <LanguageSupport.h>
#include <meta/Type.h>

namespace sys::meta
{
    /// @ingroup sys
    template <typename T>
    requires (!type<T>::is_ref())
    struct generic_nullable_adaptor
    {
    private:
        T& ref; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    public:
        constexpr /* NOLINT(hicpp-explicit-conversions) */ generic_nullable_adaptor(T& ref) noexcept : ref(ref) { }

        [[nodiscard]] constexpr bool is_null(this auto&& _this)
        requires requires {
            { _as(_this.ref, bool) } -> std::same_as<bool>;
        } || requires {
            { _this.ref } -> std::convertible_to<bool>;
        } || requires {
            { _this.ref != nullptr } -> std::convertible_to<bool>;
        }
        {
            if constexpr (requires {
                              { _as(_this.ref, bool) } -> std::same_as<bool>;
                          })
                return !_as(_this.ref, bool);
            else if constexpr (requires {
                                   { _this.ref } -> std::convertible_to<bool>;
                               })
                return !_this.ref;
            else if constexpr (requires {
                                   { _this.ref != nullptr } -> std::convertible_to<bool>;
                               })
                return !(_this.ref != nullptr);
        }
        [[nodiscard]] constexpr bool is_null(this auto&&) = delete;
    };
} // namespace sys::meta
