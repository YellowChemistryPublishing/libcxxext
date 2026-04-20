#pragma once

/// @file

#include <algorithm>
#include <tuple>

#include <LanguageSupport.h>
#include <Numeric.h>
#include <meta/Type.h>

namespace sys
{
    /// @ingroup sys
    /// @brief Uninitialized aligned storage.
    /// @tparam Ts... Align to and be able to store at least the largest of these types.
    /// @note Pass `byref`.
    template <typename... Ts>
    requires requires {
        requires sizeof...(Ts) > 0;
        requires (meta::type<Ts>::is_unqualified() && ...);
    }
    struct aligned_storage final
    {
        alignas(Ts...) byte mem[std::max({ sizeof(Ts)... })] {}; ///< Direct storage access.

        /// @brief Pointer to storage.
        template <typename T = std::tuple_element_t<0, std::tuple<Ts...>>>
        [[nodiscard]] constexpr auto* data(this auto&& _this) noexcept
        {
            return _asr(std::addressof(_this.mem), meta::replace_cv<T, decltype(_this)>*);
        }
    };
} // namespace sys
