#pragma once

/// @file CodepointIterator.h

#include <span>
#include <string>
#include <string_view>

#include <Char.h>
#include <Numeric.h>
#include <Traits.h>

namespace sys
{
    /// @brief UTF-32 codepoint view iterator for a unicode string.
    template <ICharacter T>
    struct codepoint_iter final
    {
    private:
        const T *cur = nullptr, *end = nullptr;
        ssz cp_size = 0_z;
    public:
        /// @brief Uninitialized iterator.
        constexpr codepoint_iter() = default;
        /// @brief Construct from a contiguous range.
        constexpr codepoint_iter(const T* cur, const T* end) : cur(cur), end(end) { }
        constexpr codepoint_iter(const codepoint_iter&) = default; /**< Copyable. */
        constexpr codepoint_iter(codepoint_iter&&) = default;      /**< Moveable. */
        constexpr ~codepoint_iter() = default;

        constexpr codepoint_iter& operator=(const codepoint_iter&) = default; /**< Copy-assignable. */
        constexpr codepoint_iter& operator=(codepoint_iter&&) = default;      /**< Move-assignable. */

        /// @brief Codepoint value for current position.
        constexpr char32_t operator*() noexcept
        {
            const auto [c, size] = ch::read_codepoint(std::span(this->cur, this->end), unsafe());
            this->cp_size = ssz(size);
            return c;
        }
        /// @brief Pointer to the current codepoint.
        constexpr const T* operator->() const { return this->cur; } // For `std::to_address(...)`.
        /// @brief Equality comparison.
        friend constexpr bool operator==(const codepoint_iter& a, const codepoint_iter& b) noexcept { return a.cur == b.cur && a.end == b.end; }
        /// @brief Three-way comparison.
        friend constexpr auto operator<=>(const codepoint_iter& a, const codepoint_iter& b) noexcept { return (a.cur <=> b.cur) != 0 ? a.cur <=> b.cur : a.end <=> b.end; }

        /// @brief Pre-increment.
        constexpr codepoint_iter& operator++()
        {
            if (!this->cp_size)
                this->cp_size = ssz(ch::read_codepoint(std::span(this->cur, this->end), unsafe()).second);
            this->cur += this->cp_size;
            this->cp_size = 0_z;
            return *this;
        }
        /// @brief Post-increment.
        constexpr codepoint_iter operator++(int)
        {
            const codepoint_iter ret = *this;
            ++*this;
            return ret;
        }
    };

    /// @brief UTF-32 codepoint view for a unicode string.
    template <ICharacter T>
    struct codepoint_view final
    {
    private:
        codepoint_iter<T> _beg {}, _end {};
    public:
        /// @brief Construct from a contiguous range.
        constexpr codepoint_view(const std::span<const T> range) : // NOLINT(hicpp-explicit-conversions)
            _beg(range.data(), range.data() + range.size()), _end(range.data() + range.size(), range.data() + range.size())
        { }
        constexpr codepoint_view(const codepoint_view&) = default; /**< Copyable. */
        constexpr codepoint_view(codepoint_view&&) = default;      /**< Moveable. */
        constexpr ~codepoint_view() = default;

        constexpr codepoint_view& operator=(const codepoint_view&) = default; /**< Copy-assignable. */
        constexpr codepoint_view& operator=(codepoint_view&&) = default;      /**< Move-assignable. */

        [[nodiscard]] constexpr codepoint_iter<T> begin() const { return this->_beg; } /**< Begin iterator. */
        [[nodiscard]] constexpr codepoint_iter<T> end() const { return this->_end; }   /**< End iterator. */
    };

    template <ICharacter T>
    class string;

    template <ICharacter T>
    codepoint_view(std::span<T>) -> codepoint_view<T>; /**< Deduction guide. */
    template <ICharacter T>
    codepoint_view(std::basic_string_view<T>) -> codepoint_view<T>; /**< Deduction guide. */
    template <ICharacter T>
    codepoint_view(std::basic_string<T>) -> codepoint_view<T>; /**< Deduction guide. */
    template <ICharacter T>
    codepoint_view(sys::string<T>) -> codepoint_view<T>; /**< Deduction guide. */
} // namespace sys
