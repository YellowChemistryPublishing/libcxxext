#pragma once

#include <algorithm>

#include <LanguageSupport.h>

namespace sys
{
    template <typename... Ts>
    struct aligned_storage
    {
        alignas(Ts...) byte mem[std::max({ sizeof(Ts)... })];
    };
} // namespace sys
