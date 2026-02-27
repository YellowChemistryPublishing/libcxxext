#pragma once

/// @file InplaceSet.h

#include <cstddef>
#include <utility>

namespace sys
{
    /// @brief Inplace set of data type `T` with fixed capacity.
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

        /// @brief Tries to insert a value into the set.
        /// @return Whether `value` was inserted, or the set is full.
        bool try_insert(T value)
        {
            if (this->_size == Capacity) [[unlikely]]
                return false;

            this->data[this->_size++] = std::move(value); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            return true;
        }
        /// @brief Tries to erase a value from the set.
        /// @return Whether `value` was erased, or the value was not in the set.
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
        /// @brief Checks if the set contains a value.
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
