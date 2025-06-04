#pragma once

#include "LanguageSupport.h"
#include <cmath>
#include <cstdint>
#include <initializer_list>

/* export module core.Math; */

namespace sys::math
{
    template <typename VectorData, size_t N, typename T = float>
    struct vector_operators
    {
        using ValueType = T;

        constexpr vector_operators() noexcept = default;

        constexpr T& operator[](size_t i) noexcept
        {
            return static_cast<VectorData*>(this)->elements[i];
        }
        constexpr T operator[](size_t i) const noexcept
        {
            return static_cast<const VectorData*>(this)->elements[i];
        }
        consteval static size_t size() noexcept
        {
            return N;
        }

        constexpr friend bool operator==(const VectorData& lhs, const VectorData& rhs) noexcept
        {
            for (size_t i = 0; i < N; i++)
                if (lhs[i] != rhs[i])
                    return false;
            return true;
        }

        constexpr friend VectorData operator+(const VectorData& lhs, const VectorData& rhs) noexcept
        {
            VectorData ret = lhs;
            for (size_t i = 0; i < N; i++) ret[i] += rhs[i];
            return ret;
        }
        constexpr friend VectorData operator-(const VectorData& lhs, const VectorData& rhs) noexcept
        {
            VectorData ret = lhs;
            for (size_t i = 0; i < N; i++) ret[i] -= rhs[i];
            return ret;
        }
        constexpr friend VectorData operator*(const VectorData& lhs, const VectorData& rhs) noexcept
        {
            VectorData ret = lhs;
            for (size_t i = 0; i < N; i++) ret[i] *= rhs[i];
            return ret;
        }
        constexpr friend VectorData operator*(const VectorData& lhs, T rhs) noexcept
        {
            VectorData ret = lhs;
            for (size_t i = 0; i < N; i++) ret[i] *= rhs;
            return ret;
        }
        constexpr friend VectorData operator*(T lhs, const VectorData& rhs) noexcept
        {
            VectorData ret = rhs;
            for (size_t i = 0; i < N; i++) ret[i] *= lhs;
            return ret;
        }
        constexpr friend VectorData operator/(const VectorData& lhs, const VectorData& rhs) noexcept
        {
            VectorData ret = lhs;
            for (size_t i = 0; i < N; i++) ret[i] /= rhs[i];
            return ret;
        }
        constexpr friend VectorData operator/(const VectorData& lhs, T rhs) noexcept
        {
            VectorData ret = lhs;
            for (size_t i = 0; i < N; i++) ret[i] /= rhs;
            return ret;
        }

        constexpr VectorData& operator+=(const VectorData& rhs) noexcept
        {
            for (size_t i = 0; i < N; i++) static_cast<VectorData*>(this)->elements[i] += rhs[i];
            return *static_cast<VectorData*>(this);
        }
        constexpr VectorData& operator-=(const VectorData& rhs) noexcept
        {
            for (size_t i = 0; i < N; i++) static_cast<VectorData*>(this)->elements[i] -= rhs[i];
            return *static_cast<VectorData*>(this);
        }
        constexpr VectorData& operator*=(const VectorData& rhs) noexcept
        {
            for (size_t i = 0; i < N; i++) static_cast<VectorData*>(this)->elements[i] *= rhs[i];
            return *static_cast<VectorData*>(this);
        }
        constexpr VectorData& operator*=(T rhs) noexcept
        {
            for (size_t i = 0; i < N; i++) static_cast<VectorData*>(this)->elements[i] *= rhs;
            return *static_cast<VectorData*>(this);
        }
        constexpr VectorData& operator/=(const VectorData& rhs) noexcept
        {
            for (size_t i = 0; i < N; i++) static_cast<VectorData*>(this)->elements[i] /= rhs[i];
            return *static_cast<VectorData*>(this);
        }
        constexpr VectorData& operator/=(T rhs) noexcept
        {
            for (size_t i = 0; i < N; i++) static_cast<VectorData*>(this)->elements[i] /= rhs;
            return *static_cast<VectorData*>(this);
        }

        constexpr T magnitude2() const noexcept
        {
            T ret = 0;
            for (size_t i = 0; i < N; i++) ret += static_cast<VectorData*>(this)->elements[i] * static_cast<VectorData*>(this)->elements[i];
            return ret;
        }
        constexpr T magnitude() const noexcept
        {
            if constexpr (std::is_same<T, float>::value)
                return std::sqrtf(static_cast<VectorData*>(this)->magnitude2());
            else if constexpr (std::is_same<T, double>::value)
                return std::sqrt(static_cast<VectorData*>(this)->magnitude2());
            else if constexpr (std::is_same<T, long double>::value)
                return std::sqrtl(static_cast<VectorData*>(this)->magnitude2());
            else
                return T(std::sqrt(static_cast<VectorData*>(this)->magnitude2()));
        }
    private:
        T elements[N] {};
    };

    template <typename T = float>
    struct vector2_of : public vector_operators<vector2_of<T>, 2, T>
    {
        _push_nowarn(_clWarn_pedantic);
        union
        {
            T elements[2];
            struct
            {
                T x, y;
            };
        };
        _pop_nowarn();

        using vector_operators<vector2_of<T>, 2, T>::vector_operators;

        constexpr vector2_of(T x, T y) noexcept : x(x), y(y)
        { }
    };
    template <typename T = float>
    struct vector3_of : public vector_operators<vector3_of<T>, 3, T>
    {
        _push_nowarn(_clWarn_pedantic);
        union
        {
            T elements[3];
            struct
            {
                T x, y, z;
            };
        };
        _pop_nowarn();

        using vector_operators<vector3_of<T>, 3, T>::vector_operators;

        constexpr vector3_of(T x, T y, T z) noexcept : x(x), y(y), z(z)
        { }
    };
    template <typename T = float>
    struct vector4_of : public vector_operators<vector4_of<T>, 4, T>
    {
        _push_nowarn(_clWarn_pedantic);
        union
        {
            T elements[4];
            struct
            {
                T x, y, z, w;
            };
        };
        _pop_nowarn();

        using vector_operators<vector4_of<T>, 4, T>::vector_operators;

        constexpr vector4_of(T x, T y, T z, T w) noexcept : x(x), y(y), z(z), w(w)
        { }
    };
    template <size_t N, typename T = float>
    requires (N > 4)
    struct vector_of : public vector_operators<vector_of<N, T>, N, T>
    {
        _push_nowarn(_clWarn_pedantic);
        union
        {
            T elements[N];
            struct
            {
                T x, y, z, w;
            };
        };
        _pop_nowarn();

        using vector_operators<vector_of<N, T>, N, T>::vector_operators;

        constexpr vector_of(std::initializer_list<T> elements) noexcept : elements { elements }
        { }
    };
    template <typename T>
    concept IVector = std::same_as<T, vector2_of<typename T::ValueType>> || std::same_as<T, vector3_of<typename T::ValueType>> ||
        std::same_as<T, vector4_of<typename T::ValueType>> || std::same_as<T, vector_of<T::size(), typename T::ValueType>>;

    template <IVector T>
    T dot(const T& lhs, const T& rhs) noexcept
    {
        T ret = 0;
        for (size_t i = 0; i < T::size(); i++) ret += lhs[i] * rhs[i];
        return ret;
    }
    template <typename T = float>
    vector3_of<T> cross(const vector3_of<T>& lhs, const vector3_of<T>& rhs) noexcept
    {
        return vector3_of<T> { lhs.y() * rhs.z() - lhs.z() * rhs.y(), lhs.z() * rhs.x() - lhs.x() * rhs.z(), lhs.x() * rhs.y() - lhs.y() * rhs.x() };
    }

    using vector3i8 = vector3_of<i8>;
    using vector3i16 = vector3_of<i16>;
    using vector3 = vector3_of<>;

    struct matrix3x3u8
    {
        u8 m00, m01, m02;
        u8 m10, m11, m12;
        u8 m20, m21, m22;

        constexpr bool operator==(const matrix3x3u8& rhs) const noexcept = default;
    };
} // namespace sys::math

namespace sysm = sys::math;
