#pragma once

#include <algorithm>
#include <tuple>

#include <LanguageSupport.h>

namespace sys
{
    template <typename... Ts>
    requires (sizeof...(Ts) > 0)
    struct aligned_storage
    {
        alignas(Ts...) byte mem[std::max({ sizeof(Ts)... })];

        template <typename T = std::tuple_element<0, std::tuple<Ts...>>::type>
        T* data()
        {
            return _asr(T*, std::addressof(this->mem));
        }
    };
} // namespace sys
