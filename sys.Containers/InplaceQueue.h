#pragma once

#include <cstddef>
#include <iterator>

/* export module core.Containers:InplaceQueue; */

namespace sys
{
    /// @brief A queue that stores elements in-place.
    /// @tparam T The type of elements to store.
    /// @tparam Capacity The maximum number of elements that can be stored in the queue.
    template <typename T, size_t Capacity = 128> // NOLINT(readability-magic-numbers)
    requires (Capacity > 0)
    class InplaceQueue
    {
        T data[Capacity];
        size_t _begin = 0, _end = 0;
        bool full = false;
    public:
        /// @brief Iterator for an `InplaceQueue`.
        struct Iterator
        {
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = int;
            using pointer = int*;
            using reference = int&;

            /// @brief Constructs an `Iterator` for an `InplaceQueue`.
            /// @param queue The queue to iterate over.
            /// @param i The index of the element to point to.
            Iterator(InplaceQueue& queue, size_t i) : queue(queue), i(i)
            { }

            /// @brief Dereferences the iterator.
            /// @return The element at the current index.
            reference operator*() const
            {
                return this->queue.data[this->i];
            }
            /// @brief Accesses the element at the current index.
            /// @return A pointer to the element at the current index.
            pointer operator->()
            {
                return &this->queue.data[this->i];
            }

            /// @brief Compares two iterators for equality.
            /// @param lhs The first iterator.
            /// @param rhs The second iterator.
            /// @return Whether the two iterators are equal.
            friend bool operator==(const Iterator& lhs, const Iterator& rhs)
            {
                return lhs.queue == rhs.queue && lhs.i == rhs.i;
            };

            /// @brief Prefix increments the iterator.
            /// @return A reference to the iterator after incrementing.
            Iterator& operator++()
            {
                this->i = (this->i + 1) % Capacity;
                return *this;
            }
            /// @brief Postfix increments the iterator.
            /// @return A copy of the iterator before incrementing.
            Iterator operator++(int)
            {
                Iterator ret = *this;
                this->i = (this->i + 1) % Capacity;
                return ret;
            }
            /// @brief Prefix decrements the iterator.
            /// @return A reference to the iterator after decrementing.
            Iterator& operator--()
            {
                this->i = (this->i + Capacity - 1) % Capacity;
                return *this;
            }
            /// @brief Postfix decrements the iterator.
            /// @return A copy of the iterator before decrementing.
            Iterator operator--(int)
            {
                Iterator ret = *this;
                this->i = (this->i + Capacity - 1) % Capacity;
                return ret;
            }

            friend Iterator operator+(const Iterator& a, difference_type b)
            {
                return Iterator(a.queue, (a.i + b) % Capacity);
            }
            friend Iterator operator+(difference_type a, const Iterator& b)
            {
                return Iterator(b.queue, (b.i + a) % Capacity);
            }
            friend difference_type operator-(const Iterator& a, const Iterator& b)
            {
                return a.i > b.i ? std::ptrdiff_t(a.i - b.i) : -std::ptrdiff_t(b.i - a.i);
            }
            friend Iterator operator-(const Iterator& a, difference_type b)
            {
                return Iterator(a.queue, (a.i + Capacity - b) % Capacity);
            }

            Iterator& operator+=(difference_type b)
            {
                this->i = (this->i + b) % Capacity;
                return *this;
            }
            Iterator& operator-=(difference_type b)
            {
                this->i = (this->i + Capacity - b) % Capacity;
                return *this;
            }
        private:
            InplaceQueue& queue; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
            size_t i;
        };

        InplaceQueue() noexcept = default;

        [[nodiscard]] bool empty() const noexcept
        {
            return this->size() == 0;
        }
        [[nodiscard]] size_t size() const noexcept
        {
            if (full)
                return Capacity;
            if (this->_end >= this->_begin)
                return this->_end - this->_begin;
            return Capacity - this->_begin + this->_end;
        }
        [[nodiscard]] consteval static size_t capacity() noexcept
        {
            return Capacity;
        }

        Iterator begin()
        {
            return Iterator(*this, this->_begin);
        }
        Iterator end()
        {
            return Iterator(*this, this->_end);
        }

        bool enqueue(const T& item)
        {
            if (this->full) [[unlikely]]
                return false;

            data[this->_end] = item;
            this->_end = (this->_end + 1) % Capacity;

            if (this->_end == this->_begin)
                this->full = true;

            return true;
        }
        bool tryDequeue(T& out)
        {
            if (!full && this->_begin == this->_end) [[unlikely]]
                return false;

            size_t retIdx = this->_begin;
            this->_begin = (this->_begin + 1) % Capacity;
            this->full = false;

            out = std::move(data[retIdx]);

            return true;
        }
    };
} // namespace sys
