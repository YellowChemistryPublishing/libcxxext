#pragma once

#include <LanguageSupport.h>

#include <Quaternion.h>
#include <Vector.h>

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
        constexpr static matrix4x4 translate(const vector3& v) noexcept
        {
            matrix4x4 ret;
            ret[{ 0, 3 }] = v.x;
            ret[{ 1, 3 }] = v.y;
            ret[{ 2, 3 }] = v.z;
            return ret;
        }
        constexpr static matrix4x4 scale(const vector3& v) noexcept
        {
            matrix4x4 ret;
            ret.data[0][0] = v.x;
            ret.data[1][1] = v.y;
            ret.data[2][2] = v.z;
            return ret;
        }
        constexpr static matrix4x4 rotate(const quaternion& rot) noexcept
        {
            matrix4x4 ret;

            float qxx(rot.x * rot.x);
            float qyy(rot.y * rot.y);
            float qzz(rot.z * rot.z);
            float qxz(rot.x * rot.z);
            float qxy(rot.x * rot.y);
            float qyz(rot.y * rot.z);
            float qwx(rot.w * rot.x);
            float qwy(rot.w * rot.y);
            float qwz(rot.w * rot.z);

            ret[{ 0, 0 }] = 1.0f - 2.0f * (qyy + qzz);
            ret[{ 0, 1 }] = 2.0f * (qxy + qwz);
            ret[{ 0, 2 }] = 2.0f * (qxz - qwy);

            ret[{ 1, 0 }] = 2.0f * (qxy - qwz);
            ret[{ 1, 1 }] = 1.0f - 2.0f * (qxx + qzz);
            ret[{ 1, 2 }] = 2.0f * (qyz + qwx);

            ret[{ 2, 0 }] = 2.0f * (qxz + qwy);
            ret[{ 2, 1 }] = 2.0f * (qyz - qwx);
            ret[{ 2, 2 }] = 1.0f - 2.0f * (qxx + qyy);

            return ret;
        }

        float data[4][4] { { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } };

        constexpr matrix4x4() noexcept = default;
        constexpr matrix4x4(float init) noexcept : data { { init, init, init, init }, { init, init, init, init }, { init, init, init, init }, { init, init, init, init } }
        { }
        constexpr matrix4x4(const float (&matrixData)[16]) noexcept
        {
            for (size_t i = 0; i < 4; i++)
                for (size_t j = 0; j < 4; j++) this->data[i][j] = matrixData[j * 4 + i];
        }
        constexpr matrix4x4(const matrix4x4&) noexcept = default;

        constexpr float& operator[](const size_t (&rc)[2]) noexcept
        {
            return this->data[rc[1]][rc[0]];
        }
        constexpr const float& operator[](const size_t (&rc)[2]) const noexcept
        {
            return this->data[rc[1]][rc[0]];
        }

        constexpr matrix4x4 operator+(const matrix4x4& other) const noexcept
        {
            matrix4x4 ret(*this);
            for (size_t i = 0; i < 4; i++)
                for (size_t j = 0; j < 4; j++) ret.data[i][j] = this->data[i][j] + other.data[i][j];
            return ret;
        }
        constexpr matrix4x4 operator-(const matrix4x4& other) const noexcept
        {
            matrix4x4 ret(*this);
            for (size_t i = 0; i < 4; i++)
                for (size_t j = 0; j < 4; j++) ret.data[i][j] = this->data[i][j] - other.data[i][j];
            return ret;
        }
        constexpr matrix4x4 operator*(const matrix4x4& other) const noexcept
        {
            matrix4x4 ret(0.0f);
            for (size_t i = 0; i < 4; i++)
                for (size_t j = 0; j < 4; j++)
                    for (size_t k = 0; k < 4; k++) ret[{ i, j }] += (*this)[{ i, k }] * other[{ k, j }];
            return ret;
        }
        constexpr vector4 operator*(const vector4& other) const noexcept
        {
            return vector4 { this->data[0][0] * other.x + this->data[1][0] * other.y + this->data[2][0] * other.z + this->data[3][0] * other.w,
                             this->data[0][1] * other.x + this->data[1][1] * other.y + this->data[2][1] * other.z + this->data[3][1] * other.w,
                             this->data[0][2] * other.x + this->data[1][2] * other.y + this->data[2][2] * other.z + this->data[3][2] * other.w,
                             this->data[0][3] * other.x + this->data[1][3] * other.y + this->data[2][3] * other.z + this->data[3][3] * other.w };
        }
        constexpr matrix4x4& operator+=(const matrix4x4& other) noexcept
        {
            return (*this = *this + other);
        }
        constexpr matrix4x4& operator-=(const matrix4x4& other) noexcept
        {
            return (*this = *this - other);
        }
        constexpr matrix4x4& operator*=(const matrix4x4& other) noexcept
        {
            return (*this = *this * other);
        }

        constexpr matrix4x4 transpose() const
        {
            matrix4x4 ret(*this);
            for (size_t i = 0; i < 4; i++)
                for (size_t j = 0; j < 4; j++) ret[{ i, j }] = (*this)[{ j, i }];
            return ret;
        }

        inline operator std::string() const
        {
            std::string ret;
            for (size_t i = 0; i < 4; i++)
            {
                ret.push_back('[');
                for (size_t j = 0; j < 4; j++)
                {
                    ret.append(std::to_string((*this)[{ i, j }])).append(", ");
                }
                ret.pop_back();
                ret.pop_back();
                ret.append("]\n");
            }
            ret.pop_back();
            return ret;
        }
    };
} // namespace sys::math

namespace sysm = sys::math;
