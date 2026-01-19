#pragma once

#include <cstddef>
#include <iterator>

/* export module core.Containers:inplace_queue; */

namespace sys
{
    /// @brief A queue that stores elements in-place.
    /// @tparam T The type of elements to store.
    /// @tparam Capacity The maximum number of elements that can be stored in the queue.
    template <typename T, size_t Capacity = 128> // NOLINT(readability-magic-numbers)
    requires (Capacity > 0)
    class inplace_queue
    {
        T data[Capacity];
        size_t _begin = 0, _end = 0;
        bool full = false;
    public:
        /// @brief Iterator for an `inplace_queue`.
        struct iterator
        {
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;

            /// @brief Constructs an `Iterator` for an `inplace_queue`.
            /// @param queue The queue to iterate over.
            /// @param i The index of the element to point to.
            iterator(inplace_queue& queue, size_t i) : queue(queue), i(i) { }

            /// @brief Dereferences the iterator.
            /// @return The element at the current index.
            reference operator*() const { return this->queue.data[this->i]; }
            /// @brief Accesses the element at the current index.
            /// @return A pointer to the element at the current index.
            pointer operator->() { return &this->queue.data[this->i]; }

            /// @brief Compares two iterators for equality.
            /// @param lhs The first iterator.
            /// @param rhs The second iterator.
            /// @return Whether the two iterators are equal.
            friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs.queue == rhs.queue && lhs.i == rhs.i; };

            /// @brief Prefix increments the iterator.
            /// @return A reference to the iterator after incrementing.
            iterator& operator++()
            {
                this->i = (this->i + 1) % Capacity;
                return *this;
            }
            /// @brief Postfix increments the iterator.
            /// @return A copy of the iterator before incrementing.
            iterator operator++(int)
            {
                iterator ret = *this;
                this->i = (this->i + 1) % Capacity;
                return ret;
            }
            /// @brief Prefix decrements the iterator.
            /// @return A reference to the iterator after decrementing.
            iterator& operator--()
            {
                this->i = (this->i + Capacity - 1) % Capacity;
                return *this;
            }
            /// @brief Postfix decrements the iterator.
            /// @return A copy of the iterator before decrementing.
            iterator operator--(int)
            {
                iterator ret = *this;
                this->i = (this->i + Capacity - 1) % Capacity;
                return ret;
            }

            friend iterator operator+(const iterator& a, difference_type b) { return iterator(a.queue, (a.i + b) % Capacity); }
            friend iterator operator+(difference_type a, const iterator& b) { return iterator(b.queue, (b.i + a) % Capacity); }
            friend difference_type operator-(const iterator& a, const iterator& b) { return a.i > b.i ? std::ptrdiff_t(a.i - b.i) : -std::ptrdiff_t(b.i - a.i); }
            friend iterator operator-(const iterator& a, difference_type b) { return iterator(a.queue, (a.i + Capacity - b) % Capacity); }

            iterator& operator+=(difference_type b)
            {
                this->i = (this->i + b) % Capacity;
                return *this;
            }
            iterator& operator-=(difference_type b)
            {
                this->i = (this->i + Capacity - b) % Capacity;
                return *this;
            }
        private:
            inplace_queue& queue; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
            size_t i;
        };

        inplace_queue() noexcept = default;

        [[nodiscard]] bool empty() const noexcept { return this->size() == 0; }
        [[nodiscard]] size_t size() const noexcept
        {
            if (full)
                return Capacity;
            if (this->_end >= this->_begin)
                return this->_end - this->_begin;
            return Capacity - this->_begin + this->_end;
        }
        [[nodiscard]] consteval static size_t capacity() noexcept { return Capacity; }

        iterator begin() { return iterator(*this, this->_begin); }
        iterator end() { return iterator(*this, this->_end); }

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
        bool try_dequeue(T& out)
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
