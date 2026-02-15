#pragma once

#include <span>

#include <Char.h>
#include <Numeric.h>
#include <Traits.h>

namespace sys
{
    /// @brief UTF-32 codepoint view iterator for a string of unicode characters.
    template <ICharacter T>
    struct str32_iter final
    {
    private:
        const T *cur = nullptr, *end = nullptr;
        ssz cp_size = 0_z;
    public:
        constexpr str32_iter() = default;
        constexpr str32_iter(const T* cur, const T* end) : cur(cur), end(end) { }
        constexpr str32_iter(const str32_iter&) = default;
        constexpr str32_iter(str32_iter&&) = default;
        constexpr ~str32_iter() = default;

        constexpr str32_iter& operator=(const str32_iter&) = default;
        constexpr str32_iter& operator=(str32_iter&&) = default;

        constexpr char32_t operator*() noexcept
        {
            const auto [c, size] = ch::read_codepoint(std::span(this->cur, this->end), unsafe());
            this->cp_size = ssz(size);
            return c;
        }
        constexpr const T* operator->() const { return this->cur; } // For `std::to_address(...)`.
        friend constexpr bool operator==(const str32_iter& a, const str32_iter& b) noexcept { return a.cur == b.cur && a.end == b.end; }
        friend constexpr auto operator<=>(const str32_iter& a, const str32_iter& b) noexcept { return (a.cur <=> b.cur) != 0 ? a.cur <=> b.cur : a.end <=> b.end; }

        constexpr str32_iter& operator++()
        {
            if (!this->cp_size)
                this->cp_size = ssz(ch::read_codepoint(std::span(this->cur, this->end), unsafe()).second);
            this->cur += this->cp_size;
            this->cp_size = 0_z;
            return *this;
        }
        constexpr str32_iter operator++(int)
        {
            const str32_iter ret = *this;
            ++*this;
            return ret;
        }
    };

    template <ICharacter T>
    struct str32_view final
    {
    private:
        str32_iter<T> _beg {}, _end {};
    public:
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr str32_view(const std::span<const T> range) : _beg(range.data(), range.data() + range.size()), _end(range.data() + range.size(), range.data() + range.size()) { }
        constexpr str32_view(const str32_view&) = default;
        constexpr str32_view(str32_view&&) = default;
        constexpr ~str32_view() = default;

        constexpr str32_view& operator=(const str32_view&) = default;
        constexpr str32_view& operator=(str32_view&&) = default;

        [[nodiscard]] constexpr str32_iter<T> begin() const { return this->_beg; }
        [[nodiscard]] constexpr str32_iter<T> end() const { return this->_end; }
    };
} // namespace sys
