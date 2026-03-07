#pragma once

/// @file

#include <cstddef>
#include <utility>

namespace sys
{
    /// @brief Inplace vector of fixed capacity.
    template <typename T, size_t Capacity>
    requires (Capacity > 0)
    class inplace_vector
    {
        union
        {
            T data[Capacity];
        };
        size_t _size = 0;
    public:
        /// @brief Constructs an empty vector.
        inplace_vector() = default;

        /// @brief Check vector is empty.
        [[nodiscard]] bool empty() const { return this->_size == 0; }
        /// @brief Size of vector.
        [[nodiscard]] size_t size() const { return this->_size; }
        /// @brief Capacity of vector.
        [[nodiscard]] consteval static size_t capacity() { return Capacity; }

        /// @brief Access element at index.
        [[nodiscard]] T& operator[](size_t index) { return this->data[index]; }
        /// @brief Access element at index.
        [[nodiscard]] const T& operator[](size_t index) const { return this->data[index]; }

        /// @brief Pointer to beginning.
        [[nodiscard]] T* begin() { return this->data; }
        /// @brief Pointer to end.
        [[nodiscard]] T* end() { return this->data + this->_size; }

        /// @brief Pushes a value into the vector.
        /// @return Whether `value` was pushed, or the vector is full.
        [[nodiscard]] bool push_back(T value)
        {
            if (this->_size >= Capacity)
                return false; // NOLINT(readability-simplify-boolean-expr)
            new(&this->data[this->_size++]) T(std::move(value));
            return true;
        }
        /// @brief Pops the last element from the vector.
        void pop_back()
        {
            if (this->_size > 0) [[likely]]
                this->data[this->_size--].~T();
        }
        /// @brief Clears the vector.
        void clear()
        {
            for (size_t i = 0; i < this->_size; ++i)
                this->data[i].~T();
            this->_size = 0;
        }

        /// @brief Finds the first occurrence of `value` in the vector.
        /// @return Pointer to the first occurrence of `value`, or `nullptr` if not found.
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
