#pragma once

#include <LanguageSupport.h>

namespace sys
{
    template <typename T>
    struct recurring_template
    {
    protected:
        using recurring_type = T;

        recurring_template() = default;

        constexpr auto* downcast(this auto&& _this) noexcept
        {
            using downcasted_this_type = meta::replace_cv<recurring_type, std::remove_reference_t<decltype(_this)>>&;
            return &_as(downcasted_this_type, _this);
        }
    };
} // namespace sys
