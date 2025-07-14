#pragma once

#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <sstream>

#include <LanguageSupport.h>

/* export module core.Math; */

namespace sys::math
{
    template <std::floating_point T>
    struct quaternion_of;

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

        constexpr VectorData operator-() const noexcept
        {
            VectorData ret = static_cast<const VectorData&>(*this);
            for (size_t i = 0; i < N; i++) ret[i] = -ret[i];
            return ret;
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

        constexpr T magnitude_sq() const noexcept
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

        friend std::ostream& operator<<(std::ostream& stream, const vector_operators& vec)
        {
            stream << '(' << vec[0];
            for (size_t i = 1u; i < vec.size(); i++) stream << ", " << vec[i];
            stream << ')';
            return stream;
        }
        friend std::wostream& operator<<(std::wostream& stream, const vector_operators& vec)
        {
            stream << L'(' << vec[0];
            for (size_t i = 1u; i < vec.size(); i++) stream << L", " << vec[i];
            stream << L')';
            return stream;
        }
        friend std::string to_string(const vector_operators& vec)
        {
            std::ostringstream sstr;
            sstr << vec;
            return std::move(sstr).str();
        }
    };

    template <typename T = float>
    struct vector2_of : public vector_operators<vector2_of<T>, 2, T>
    {
        static const vector2_of<T> zero;
        static const vector2_of<T> one;

        _push_nowarn_gcc(_clWarn_gcc_pedantic);
        _push_nowarn_clang(_clWarn_clang_pedantic);
        _push_nowarn_clang(_clWarn_clang_nameless_struct_union);
        _push_nowarn_msvc(_clWarn_msvc_nameless_struct_union);
        union
        {
            T elements[2];
            struct
            {
                T x, y;
            };
        };
        _pop_nowarn_msvc();
        _pop_nowarn_clang();
        _pop_nowarn_clang();
        _pop_nowarn_gcc();

        using vector_operators<vector2_of<T>, 2, T>::vector_operators;

        constexpr vector2_of(T init) noexcept : x(init), y(init)
        { }
        constexpr vector2_of(T x, T y) noexcept : x(x), y(y)
        { }
    };
    template <typename T = float>
    struct vector3_of : public vector_operators<vector3_of<T>, 3, T>
    {
        static const vector3_of<T> zero;
        static const vector3_of<T> one;

        static const vector3_of<T> up;
        static const vector3_of<T> right;
        static const vector3_of<T> forward;

        _push_nowarn_gcc(_clWarn_gcc_pedantic);
        _push_nowarn_clang(_clWarn_clang_pedantic);
        _push_nowarn_clang(_clWarn_clang_nameless_struct_union);
        _push_nowarn_msvc(_clWarn_msvc_nameless_struct_union);
        union
        {
            T elements[3];
            struct
            {
                T x, y, z;
            };
        };
        _pop_nowarn_msvc();
        _pop_nowarn_clang();
        _pop_nowarn_clang();
        _pop_nowarn_gcc();

        using vector_operators<vector3_of<T>, 3, T>::vector_operators;

        constexpr vector3_of(T init) noexcept : x(init), y(init), z(init)
        { }
        constexpr vector3_of(T x, T y, T z) noexcept : x(x), y(y), z(z)
        { }

        template <std::floating_point Floating>
        requires std::same_as<T, Floating>
        constexpr vector3_of rotate(const quaternion_of<Floating>& rot) const noexcept;
    };
    template <typename T = float>
    struct vector4_of : public vector_operators<vector4_of<T>, 4, T>
    {
        static const vector4_of<T> zero;
        static const vector4_of<T> one;

        _push_nowarn_gcc(_clWarn_gcc_pedantic);
        _push_nowarn_clang(_clWarn_clang_pedantic);
        _push_nowarn_clang(_clWarn_clang_nameless_struct_union);
        _push_nowarn_msvc(_clWarn_msvc_nameless_struct_union);
        union
        {
            T elements[4];
            struct
            {
                T x, y, z, w;
            };
        };
        _pop_nowarn_msvc();
        _pop_nowarn_clang();
        _pop_nowarn_clang();
        _pop_nowarn_gcc();

        using vector_operators<vector4_of<T>, 4, T>::vector_operators;

        constexpr vector4_of(T init) noexcept : x(init), y(init), z(init), w(init)
        { }
        constexpr vector4_of(T x, T y, T z, T w) noexcept : x(x), y(y), z(z), w(w)
        { }
    };
    template <size_t N, typename T = float>
    requires (N > 4)
    struct vector_of : public vector_operators<vector_of<N, T>, N, T>
    {
        _push_nowarn_gcc(_clWarn_gcc_pedantic);
        _push_nowarn_clang(_clWarn_clang_pedantic);
        _push_nowarn_clang(_clWarn_clang_nameless_struct_union);
        _push_nowarn_msvc(_clWarn_msvc_nameless_struct_union);
        union
        {
            T elements[N];
            struct
            {
                T x, y, z, w;
            };
        };
        _pop_nowarn_msvc();
        _pop_nowarn_clang();
        _pop_nowarn_clang();
        _pop_nowarn_gcc();

        using vector_operators<vector_of<N, T>, N, T>::vector_operators;

        constexpr vector_of(std::initializer_list<T> elements) noexcept : elements { elements }
        { }
    };

    template <typename T>
    constexpr vector2_of<T> vector2_of<T>::zero = vector2_of<T>(0, 0);
    template <typename T>
    constexpr vector2_of<T> vector2_of<T>::one = vector2_of<T>(1, 1);

    template <typename T>
    constexpr vector3_of<T> vector3_of<T>::zero = vector3_of<T>(0, 0, 0);
    template <typename T>
    constexpr vector3_of<T> vector3_of<T>::one = vector3_of<T>(1, 1, 1);
    template <typename T>
    constexpr vector3_of<T> vector3_of<T>::up = vector3_of<T>(0, 1, 0);
    template <typename T>
    constexpr vector3_of<T> vector3_of<T>::right = vector3_of<T>(1, 0, 0);
    template <typename T>
    constexpr vector3_of<T> vector3_of<T>::forward = vector3_of<T>(0, 0, 1);

    template <typename T>
    constexpr vector4_of<T> vector4_of<T>::zero = vector4_of<T>(0, 0, 0, 0);
    template <typename T>
    constexpr vector4_of<T> vector4_of<T>::one = vector4_of<T>(1, 1, 1, 1);

    template <typename T>
    concept IVector = std::same_as<T, vector2_of<typename T::ValueType>> || std::same_as<T, vector3_of<typename T::ValueType>> ||
        std::same_as<T, vector4_of<typename T::ValueType>> || std::same_as<T, vector_of<T::size(), typename T::ValueType>>;

    template <template <typename> class Vector, typename T>
    T dot(const Vector<T>& a, const Vector<T>& b) noexcept
    {
        T ret = 0;
        for (size_t i = 0; i < Vector<T>::size(); i++) ret += a[i] * b[i];
        return ret;
    }
    template <typename T = float>
    vector3_of<T> cross(const vector3_of<T>& lhs, const vector3_of<T>& rhs) noexcept
    {
        return vector3_of<T> { lhs.y() * rhs.z() - lhs.z() * rhs.y(), lhs.z() * rhs.x() - lhs.x() * rhs.z(), lhs.x() * rhs.y() - lhs.y() * rhs.x() };
    }

    using vector2 = vector2_of<>;
    using vector2i32 = vector2_of<i32>;

    using vector3i8 = vector3_of<i8>;
    using vector3i16 = vector3_of<i16>;
    using vector3 = vector3_of<>;
    using vector3i32 = vector3_of<i32>;

    using vector4 = vector4_of<>;
} // namespace sys::math

namespace sysm = sys::math;
