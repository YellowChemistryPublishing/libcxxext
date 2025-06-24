#pragma once

#include <LanguageSupport.h>

namespace sys::math
{
    struct matrix3x3u8
    {
        u8 m00, m01, m02;
        u8 m10, m11, m12;
        u8 m20, m21, m22;

        constexpr bool operator==(const matrix3x3u8& rhs) const noexcept = default;
    };
    struct matrix4x4
    {
        float m00, m01, m02, m03;
        float m10, m11, m12, m13;
        float m20, m21, m22, m23;
        float m30, m31, m32, m33;

        constexpr bool operator==(const matrix4x4& rhs) const noexcept = default;
    };
}

namespace sysm = sys::math;
