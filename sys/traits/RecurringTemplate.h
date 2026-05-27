#pragma once

/// @file

#include <type_traits>

#include <LanguageSupport.h>
#include <meta/Type.h>

namespace sys::traits
{
    /// @ingroup sys
    /// @brief Helper inheritable for accessing derived type `T`.
    /// @details
    /// Implements `sys::INothrowDefaultConstructible`, `sys::INothrowCopyConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowCopyAssignable`,
    /// `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`.
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    struct recurring_template
    {
    protected:
        using recurring_type = T; ///< @private

        /// @cond

        recurring_template() noexcept = default;                          // NOLINT(bugprone-crtp-constructor-accessibility)
        recurring_template(const recurring_template&) noexcept = default; // NOLINT(bugprone-crtp-constructor-accessibility)
        recurring_template(recurring_template&&) noexcept = default;      // NOLINT(bugprone-crtp-constructor-accessibility)
        ~recurring_template() noexcept = default;

        recurring_template& operator=(const recurring_template&) noexcept = default;
        recurring_template& operator=(recurring_template&&) noexcept = default;

        /// @endcond

        /// @private
        /// @brief Downcasts to `T`.
        constexpr auto downcast(this auto&& _this) noexcept -> meta::replace_cv<T, decltype(_this)>& // LCOV_EXCL_LINE
        {
            return _as(_this, meta::replace_cv<T, decltype(_this)>&); // LCOV_EXCL_LINE
        }
    };
} // namespace sys::traits
