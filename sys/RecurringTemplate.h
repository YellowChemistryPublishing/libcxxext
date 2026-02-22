#pragma once

#include <type_traits>

#include <LanguageSupport.h>
#include <Traits.h>

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
            using type = meta::replace_cv<recurring_type, std::remove_reference_t<decltype(_this)>>;
            return _as(type*, std::addressof(_this));
        }
    };
} // namespace sys
