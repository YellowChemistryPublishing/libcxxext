#pragma once

/// @file

#include <type_traits>

#include <LanguageSupport.h>
#include <meta/Type.h>

namespace sys
{
    /// @brief Helper inheritable for accessing derived type `T`.
    template <typename T>
    struct recurring_template
    {
    protected:
        using recurring_type = T; ///< @private

        /// @private
        recurring_template() = default;

        /// @private
        /// @brief Downcasts to `T`.
        constexpr auto& downcast(this auto&& _this) noexcept
        {
            using type = meta::replace_cv<recurring_type, std::remove_reference_t<decltype(_this)>>;
            return *_as(type*, std::addressof(_this));
        }
    };
} // namespace sys
