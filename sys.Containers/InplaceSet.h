#pragma once

#include <cstddef>
#include <utility>

namespace sys
{
    template <typename T, size_t Capacity>
    requires (Capacity > 0)
    class inplace_set
    {
        T data[Capacity] {};
        size_t _size = 0;
    public:
        inplace_set() = default;

        [[nodiscard]] size_t size() const { return this->_size; }
        [[nodiscard]] consteval static size_t capacity() { return Capacity; }

        bool try_insert(T value)
        {
            if (this->_size == Capacity) [[unlikely]]
                return false;

            this->data[this->_size++] = std::move(value); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            return true;
        }
        bool try_erase(T value)
        {
            for (size_t i = 0; i < this->_size; i++)
            {
                if (this->data[i] == value) // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
                {
                    this->data[i] = std::move(this->data[--this->_size]); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
                    return true;
                }
            }
            return false;
        }
        bool contains(T value)
        {
            for (size_t i = 0; i < this->_size; i++)
            {
                if (this->data[i] == value)
                    return true;
            }
            return false;
        }
    };
} // namespace sys
