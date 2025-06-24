#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>

namespace sys::math
{
    template <std::floating_point T>
    constexpr T clamp(T val, std::type_identity_t<T> a, std::type_identity_t<T> b)
    {
        return std::max(std::min(val, b), a);
    }
} // namespace sys::math

namespace sysm = sys::math;
