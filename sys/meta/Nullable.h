#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

#include <LanguageSupport.h>

namespace sys::meta
{
    template <typename T>
    requires (!std::is_reference_v<T>)
    struct generic_nullable_adaptor
    {
    private:
        T& ref; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    public:
        constexpr /* NOLINT(hicpp-explicit-conversions) */ generic_nullable_adaptor(T& ref) noexcept : ref(ref) { }

        [[nodiscard]] constexpr bool is_null()
        requires requires {
            { _as(bool, this->ref) } -> std::same_as<bool>;
        } || requires {
            { this->ref } -> std::convertible_to<bool>;
        } || requires {
            { this->ref != nullptr } -> std::convertible_to<bool>;
        }
        {
            if constexpr (requires {
                              { _as(bool, this->ref) } -> std::same_as<bool>;
                          })
                return !_as(bool, this->ref);
            else if constexpr (requires {
                                   { this->ref } -> std::convertible_to<bool>;
                               })
                return !this->ref;
            else if constexpr (requires {
                                   { this->ref != nullptr } -> std::convertible_to<bool>;
                               })
                return !(this->ref != nullptr);
            else
                std::unreachable();
        }
    };
} // namespace sys::meta
