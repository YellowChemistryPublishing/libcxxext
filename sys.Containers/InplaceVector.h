#pragma once

#include <cstddef>
#include <utility>

namespace sys
{
    template <typename T, size_t Capacity>
    requires (Capacity > 0)
    class InplaceVector
    {
        union
        {
            T data[Capacity];
        };
        size_t _size = 0;
    public:
        InplaceVector() = default;

        [[nodiscard]] bool empty() const
        {
            return this->_size == 0;
        }
        [[nodiscard]] size_t size() const
        {
            return this->_size;
        }
        [[nodiscard]] consteval static size_t capacity()
        {
            return Capacity;
        }

        [[nodiscard]] T& operator[](size_t index)
        {
            return this->data[index];
        }
        [[nodiscard]] const T& operator[](size_t index) const
        {
            return this->data[index];
        }

        [[nodiscard]] T* begin()
        {
            return this->data;
        }
        [[nodiscard]] T* end()
        {
            return this->data + this->_size;
        }

        [[nodiscard]] bool pushBack(T value)
        {
            if (this->_size >= Capacity)
                return false; // NOLINT(readability-simplify-boolean-expr)
            new(&this->data[this->_size++]) T(std::move(value));
            return true;
        }
        void popBack()
        {
            if (this->_size > 0) [[likely]]
                this->data[this->_size--].~T();
        }
        void clear()
        {
            for (size_t i = 0; i < this->_size; ++i) this->data[i].~T();
            this->_size = 0;
        }

        [[nodiscard]] T* find(const T& value)
        {
            for (size_t i = 0; i < this->_size; i++)
            {
                if (this->data[i] == value)
                    return &this->data[i];
            }
            return nullptr;
        }
    };
} // namespace sys
