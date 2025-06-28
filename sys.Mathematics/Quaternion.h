#pragma once

#include <concepts>

#include <Vector.h>

namespace sys::math
{
    template <std::floating_point T = float>
    struct quaternion_of
    {
        static const quaternion_of identity;

        inline static quaternion_of fromEuler(vector3_of<T> r) noexcept
        {
            // Shamelessly "inspired" by the glm source for their Quaternions.
            // Modified, see: https://github.com/g-truc/glm, under MIT.
            vector3_of<T> c(cosf(r.x * 0.5), cosf(r.y * 0.5), cosf(r.z * 0.5));
            vector3_of<T> s(sinf(r.x * 0.5), sinf(r.y * 0.5), sinf(r.z * 0.5));

            return quaternion_of { c.x * s.y * c.z + s.x * c.y * s.z, s.x * c.y * c.z - c.x * s.y * s.z, c.x * c.y * s.z - s.x * s.y * c.z, c.x * c.y * c.z + s.x * s.y * s.z };
        }

        T x = 0.0, y = 0.0, z = 0.0, w = 1.0;

        constexpr quaternion_of() noexcept = default;
        constexpr quaternion_of(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w)
        { }
        constexpr quaternion_of(float real, vector3_of<T> right) noexcept : x(right.x), y(right.y), z(right.z), w(real)
        { }
        constexpr quaternion_of(const quaternion_of&) noexcept = default;

        constexpr quaternion_of operator*(float other) const noexcept
        {
            return quaternion_of(this->x * other, this->y * other, this->z * other, this->w * other);
        }
        constexpr quaternion_of operator/(float other) const noexcept
        {
            return quaternion_of(this->x / other, this->y / other, this->z / other, this->w / other);
        }
        constexpr quaternion_of operator*(const quaternion_of& other) const noexcept
        {
            return quaternion_of { this->w * other.x + this->x * other.w + this->y * other.z - this->z * other.y,
                                   this->w * other.y - this->x * other.z + this->y * other.w + this->z * other.x,
                                   this->w * other.z + this->x * other.y - this->y * other.x + this->z * other.w,
                                   this->w * this->x - this->x * other.x - this->y * other.y - this->z * other.z };
        }
        constexpr quaternion_of& operator*=(float other) noexcept
        {
            this->x *= other;
            this->y *= other;
            this->z *= other;
            this->w *= other;
            return *this;
        }
        friend constexpr quaternion_of operator*(float lhs, const quaternion_of& rhs) noexcept
        {
            return quaternion_of(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
        }
        constexpr quaternion_of& operator*=(const quaternion_of& other) noexcept
        {
            *this = *this * other;
            return *this;
        }
        constexpr quaternion_of& operator/=(float other) noexcept
        {
            this->x /= other;
            this->y /= other;
            this->z /= other;
            this->w /= other;
            return *this;
        }

        constexpr bool is_scalar() const noexcept
        {
            return this->x == 0.0 && this->y == 0.0 && this->z == 0.0;
        }
        constexpr bool is_vector() const noexcept
        {
            return this->w == 0.0;
        }

        constexpr float scalar() const noexcept
        {
            return this->w;
        }
        constexpr vector3 vector() const noexcept
        {
            return vector3(this->x, this->y, this->z);
        }

        constexpr quaternion_of inverse() const noexcept
        {
            // vector3 v = this->vector();
            // float d = (this->w * this->w + ::dot(v, v));
            // return quaternion_of(this->w / d, -v / d);
            // I don't know if the below is better but we're keeping both just in case.
            return this->conjugate() / this->norm2();
        }
        constexpr quaternion_of conjugate() const noexcept
        {
            return quaternion_of(-this->x, -this->y, -this->z, this->w);
        }
        constexpr float norm2() const noexcept
        {
            return this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w;
        }
        inline float norm() const noexcept
        {
            return sqrtf(this->norm2());
        }
        inline quaternion_of unit()
        {
            return *this / this->norm();
        }
        inline quaternion_of normalized()
        {
            return this->unit();
        }

        inline operator std::string() const
        {
            std::string ret;
            ret.push_back('[');
            ret.append(std::to_string(this->w))
                .append(", (")
                .append(std::to_string(this->x))
                .append(", ")
                .append(std::to_string(this->y))
                .append(", ")
                .append(std::to_string(this->z))
                .append(")]");
            return ret;
        }
    };

    template <std::floating_point T>
    constexpr quaternion_of<T> quaternion_of<T>::identity(0.0, 0.0, 0.0, 1.0);

    using quaternion = quaternion_of<>;

    template <typename T>
    template <std::floating_point Floating>
    requires std::same_as<T, Floating>
    constexpr vector3_of<T> vector3_of<T>::rotate(const quaternion_of<Floating>& rot) const noexcept
    {
        /// FIXME: Is this a hack? Why do we need operator- in front?
        return -(rot * quaternion_of<Floating>(0.0, *this) * rot.conjugate()).vector();
    }
} // namespace sys::math

namespace sysm = sys::math;
