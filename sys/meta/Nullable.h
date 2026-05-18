#pragma once

/// @file

#include <concepts>

#include <LanguageSupport.h>
#include <meta/Type.h>

namespace sys::internal
{
    template <typename T>
    concept IAsBool = requires(T t) {
        { _as(t, bool) } -> std::same_as<bool>;
    };
    template <typename T>
    concept IConvToBool = requires(T t) {
        { !t } -> std::convertible_to<bool>;
    };
    template <typename T>
    concept INotEqNullptr = requires(T t) {
        { t != nullptr } -> std::convertible_to<bool>;
    };
} // namespace sys::internal

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
        requires internal::IAsBool<T&> || internal::IConvToBool<T&> || internal::INotEqNullptr<T&>
        {
            if constexpr (internal::IAsBool<T&>)
                return !_as(_this.ref, bool);
            else if constexpr (internal::IConvToBool<T&>)
                return !_this.ref;
            else if constexpr (internal::INotEqNullptr<T&>)
                return !(_this.ref != nullptr);
        }
        [[nodiscard]] constexpr bool is_null(this auto&&) = delete;
    };

    template <typename T>
    generic_nullable_adaptor(const T&) -> generic_nullable_adaptor<const T>;
    template <typename T>
    generic_nullable_adaptor(T&) -> generic_nullable_adaptor<T>;
} // namespace sys::meta
