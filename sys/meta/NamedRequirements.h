#pragma once

#include <type_traits>

namespace sys
{
    template <typename T>
    concept ISwappable = std::is_swappable_v<T>;
    template <typename T>
    concept INothrowSwappable = std::is_nothrow_swappable_v<T>;
}; // namespace sys
