#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <limits>
#include <utility>

#include <LanguageSupport.h>
#include <Pointer.h>
#include <Result.h>

namespace sys
{
    template <typename T>
    struct managed_array
    {
        inline managed_array(ssz)
        {
            _assert_ctor_can_fail();
        }
        [[nodiscard]] _const inline static result<managed_array<T>> ctor(ssz len)
        {
            if (len < 0 || len > std::numeric_limits<size_t>::max())
                return nullptr;

            managed_array<T> ret;
            ret.data = new T[+len];
            ret._length = len;
            return ret;
        }
        inline managed_array(ssz, T)
        {
            _assert_ctor_can_fail();
        }
        [[nodiscard]] _const inline static result<managed_array<T>> ctor(ssz len, T init)
        {
            if (len < 0 || len > std::numeric_limits<size_t>::max())
                return nullptr;

            managed_array<T> ret;
            sc_ptr<T[]> incomplete = new T[+len];
            for (auto it = &*incomplete; it != &*incomplete + len; ++it) *it = init;
            ret.data = incomplete.move();
            ret._length = len;
            return ret;
        }
        inline managed_array(std::initializer_list<T> init)
        {
            sc_ptr<T[]> incomplete = new T[init.size()];
            for (auto it1 = &*incomplete, it2 = init.begin(); it1 != &*incomplete + init.size(); ++it1, ++it2) *it1 = *it2;
            this->data = incomplete.move();
            this->_length = init.size();
        }
        inline managed_array(const managed_array& other)
        {
            sc_ptr<T[]> incomplete = new T[+other._length];
            for (auto it1 = &*incomplete, it2 = other.begin(); it2 != other.end(); ++it1, ++it2) *it1 = *it2;
            this->data = incomplete.move();
            this->_length = other._length;
        }
        inline managed_array(managed_array&& other) noexcept
        {
            swap(*this, other);
        }
        inline ~managed_array()
        {
            delete[] this->data;
        }

        _const inline managed_array& operator=(const managed_array& other)
        {
            sc_ptr<T[]> incomplete = new T[+other._length];
            for (auto it1 = &*incomplete, it2 = other.begin(); it2 != other.end(); ++it1, ++it2) *it1 = *it2;
            delete[] this->data;
            this->data = incomplete.move();
            this->_length = other._length;
            return *this;
        }
        _const inline managed_array& operator=(managed_array&& other) noexcept
        {
            this->_length = 0;
            delete[] this->data;
            this->data = nullptr;
            swap(*this, other);
            return *this;
        }

        _const inline const T& operator[](ssz index, unsafe) const noexcept
        {
            return this->data[+index];
        }
        _const inline T& operator[](ssz index, unsafe) noexcept
        {
            return _asc(T&, (*_as(const std::remove_reference_t<decltype(*this)>*, this))[index, unsafe()]);
        }
        _const inline result<const T&> operator[](ssz index) const noexcept
        {
            if (ssz(0) <= index && index < this->_length) [[likely]]
                return (*this)[index, unsafe()];
            else
                return nullptr;
        }
        _const inline result<T&> operator[](ssz index) noexcept
        {
            if (ssz(0) <= index && index < this->_length) [[likely]]
                return (*this)[index, unsafe()];
            else
                return nullptr;
        }

        _const inline const T* cbegin() const noexcept
        {
            return this->data;
        }
        _const inline const T* cend() const noexcept
        {
            return this->data + (+this->_length);
        }
        _const inline const T* begin() const noexcept
        {
            return this->cbegin();
        }
        _const inline const T* end() const noexcept
        {
            return this->cend();
        }
        _const inline T* begin() noexcept
        {
            return this->data;
        }
        _const inline T* end() noexcept
        {
            return this->data + (+this->_length);
        }

        _const inline bool is_empty() const noexcept
        {
            return this->_length == 0;
        }
        _const inline ssz length() const noexcept
        {
            return this->_length;
        }
        _const inline size_t size() const noexcept
        {
            return this->length();
        }
        _const inline ssz capacity() const noexcept
        {
            return this->_length;
        }
        inline void clear()
        {
            delete[] this->data;
            this->data = nullptr;
            this->_length = 0;
        }

        friend inline void swap(sys::managed_array<T>& a, sys::managed_array<T>& b) noexcept
        {
            using std::swap;

            swap(a.data, b.data);
            swap(a._length, b._length);
        }
    private:
        T* data = nullptr;
        ssz _length = 0;

        inline managed_array()
        { }
    };
} // namespace sys
