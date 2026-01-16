#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include <LanguageSupport.h>

namespace sys
{
    template <typename T>
    struct sc_ptr_b
    {
        constexpr sc_ptr_b() noexcept = default;
        constexpr sc_ptr_b(T* ptr) noexcept : _ptr(ptr) // NOLINT(hicpp-explicit-conversions)
        { }
        constexpr sc_ptr_b(const sc_ptr_b&) = delete;
        constexpr sc_ptr_b(sc_ptr_b&& other) noexcept : _ptr(other._ptr)
        {
            swap(*this, other);
        }
        constexpr ~sc_ptr_b()
        {
            delete this->_ptr;
        }

        constexpr sc_ptr_b& operator=(const sc_ptr_b&) = delete;
        constexpr sc_ptr_b& operator=(sc_ptr_b&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        T& operator*() const noexcept
        {
            return *this->_ptr;
        }
        T* operator->() const noexcept
        {
            return this->_ptr;
        }
        T& operator[](ssz index) const noexcept
        {
            return *(this->_ptr + index);
        }

        T* move() noexcept
        {
            T* ret = this->_ptr;
            this->_ptr = nullptr;
            return ret;
        }

        friend void swap(sc_ptr_b& a, sc_ptr_b& b) noexcept
        {
            using std::swap;
            swap(a._ptr, b._ptr);
        }
    protected:
        T* _ptr = nullptr; // NOLINT(misc-non-private-member-variables-in-classes)
    };

    template <typename T>
    struct sc_ptr : public sc_ptr_b<T>
    {
        using sc_ptr_b<T>::sc_ptr_b;
        sc_ptr(const sc_ptr&) = delete;
        sc_ptr(sc_ptr&& other) noexcept
        {
            swap(_as(sc_ptr_b<T>&, *this), _as(sc_ptr_b<T>&, other));
        }
        ~sc_ptr()
        {
            delete this->_ptr;
        }

        sc_ptr& operator=(const sc_ptr&) = delete;
        sc_ptr& operator=(sc_ptr&& other) noexcept
        {
            swap(_as(sc_ptr_b<T>&, *this), _as(sc_ptr_b<T>&, other));
            return *this;
        }
    };
    template <typename T>
    struct sc_ptr<T[]> : public sc_ptr_b<T>
    {
        using sc_ptr_b<T>::sc_ptr_b;
        sc_ptr(const sc_ptr&) = delete;
        sc_ptr(sc_ptr&& other) noexcept
        {
            swap(_as(sc_ptr_b<T>&, *this), _as(sc_ptr_b<T>&, other));
        }
        ~sc_ptr()
        {
            delete[] this->_ptr;
        }

        sc_ptr& operator=(const sc_ptr&) = delete;
        sc_ptr& operator=(sc_ptr&& other) noexcept
        {
            swap(_as(sc_ptr_b<T>&, *this), _as(sc_ptr_b<T>&, other));
            return *this;
        }
    };

    template <typename Func>
    struct sc_act
    {
        constexpr sc_act(Func func) noexcept : func(std::move(func)) // NOLINT(hicpp-explicit-conversions)
        { }
        constexpr sc_act(const sc_act&) = delete;
        constexpr sc_act(sc_act&& other) noexcept
        {
            swap(*this, other);
        }
        ~sc_act()
        {
            if (!this->released)
                std::invoke(func);
        }

        sc_act& operator=(const sc_act&) = delete;
        sc_act& operator=(sc_act&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        void release() noexcept
        {
            this->released = true;
        }

        friend void swap(sc_act& a, sc_act& b) noexcept(std::is_nothrow_swappable_v<sc_act>)
        {
            using std::swap;

            swap(a.func, b.func);
            swap(a.released, b.released);
        }
    private:
        _no_unique_address Func func;
        bool released = false;
    };
} // namespace sys
