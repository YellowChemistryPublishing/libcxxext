#pragma once

#include <algorithm>
#include <tuple>

#include <Integer.h>
#include <LanguageSupport.h>

namespace sys
{
    /// @brief Uninitialized aligned storage.
    ///
    /// @tparam Ts... Align to and be able to store at least the largest of these types.
    /// @note Pass `byref`.
    template <typename... Ts>
    requires (sizeof...(Ts) > 0)
    struct aligned_storage
    {
        alignas(Ts...) byte mem[std::max({ sizeof(Ts)... })] {};

        template <typename T = std::tuple_element_t<0, std::tuple<Ts...>>>
        [[nodiscard]] const T* data() const noexcept
        {
            return _asr(const T*, std::addressof(this->mem));
        }
        template <typename T = std::tuple_element_t<0, std::tuple<Ts...>>>
        [[nodiscard]] T* data() noexcept
        {
            return _asr(T*, std::addressof(this->mem));
        }
    };
} // namespace sys
