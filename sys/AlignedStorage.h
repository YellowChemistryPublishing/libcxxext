#pragma once

/// @file

#include <algorithm>
#include <concepts>

#include <LanguageSupport.h>
#include <meta/Type.h>

namespace sys
{
    /// @ingroup sys
    /// @brief Uninitialized aligned storage.
    /// @tparam Ts... Align to and be able to store at least the largest of these types.
    /// @note Pass `byref`.
    /// @details Implements `sys::INothrowDefaultConstructible`, `sys::INothrowDestructible`.
    template <typename... Ts>
    requires requires {
        requires sizeof...(Ts) > 0;
        requires (meta::type<Ts>::is_unqualified() && ...);
    }
    struct aligned_storage final
    {
    private:
        alignas(Ts...) byte mem[std::max({ sizeof(Ts)... })] {}; ///< Direct storage access.
    public:
        constexpr aligned_storage() noexcept = default;
        constexpr aligned_storage(const aligned_storage&) noexcept = delete;
        constexpr aligned_storage(aligned_storage&&) noexcept = delete;
        constexpr ~aligned_storage() noexcept = default;

        constexpr aligned_storage& operator=(const aligned_storage&) noexcept = delete;
        constexpr aligned_storage& operator=(aligned_storage&&) noexcept = delete;

        /// @brief Pointer to storage.
        template <typename T = meta::parameter_pack<Ts...>::template at<0>>
        requires (meta::parameter_pack<Ts...>::template contains<T>() || std::same_as<T, void>)
        [[nodiscard]] constexpr auto* data(this auto&& _this) noexcept
        {
            return _asr(std::addressof(_this.mem), meta::replace_cv<T, decltype(_this)>*);
        }
    };
} // namespace sys
