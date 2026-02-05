#pragma once

#include <cstddef>
#include <iterator>
#include <string>

#include <LanguageSupport.h>
#include <Traits.h>

namespace sys
{
    template <ICharacter T>
    struct [[nodiscard]] alignas(const T*) strlit final
    {
        struct iter final
        {
        private:
            const T* ptr = nullptr;

            _inline_always constexpr explicit iter(const T* p) noexcept : ptr(p) { }
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = const T*;
            using reference = T;

            _inline_always constexpr iter() noexcept = default;
            _inline_always constexpr iter(const iter& it) noexcept = default;
            _inline_always constexpr iter(iter&& it) noexcept = default;
            _inline_always constexpr ~iter() noexcept = default;

            _inline_always constexpr iter& operator=(const iter& it) noexcept = default;
            _inline_always constexpr iter& operator=(iter&& it) noexcept = default;

            [[nodiscard]] _inline_always constexpr reference operator*() const noexcept { return *this->ptr; }
            [[nodiscard]] _inline_always constexpr pointer operator->() const noexcept { return this->ptr; }

            _inline_always friend constexpr bool operator==(iter a, iter b) noexcept { return (a.ptr && *a.ptr ? a.ptr : nullptr) == (b.ptr && *b.ptr ? b.ptr : nullptr); }
            _inline_always friend constexpr auto operator<=>(iter a, iter b) noexcept = default;

            _inline_always constexpr iter& operator++() noexcept
            {
                ++this->ptr; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return *this;
            }
            _inline_always constexpr iter operator++(int) noexcept
            {
                iter ret = *this;
                ++this->ptr; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return ret;
            }
            _inline_always constexpr iter& operator--() noexcept
            {
                --this->ptr; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return *this;
            }
            _inline_always constexpr iter operator--(int) noexcept
            {
                iter ret = *this;
                --this->ptr; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return ret;
            }

            friend struct sys::strlit<T>;
        };
    private:
        const T* lit = nullptr;
    public:
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        consteval strlit(const T* str) noexcept : lit(str) { }
        constexpr strlit(const strlit& other) noexcept = default;
        constexpr strlit(strlit&& other) noexcept = default;
        constexpr ~strlit() noexcept = default;

        constexpr strlit& operator=(const strlit& other) noexcept = default;
        constexpr strlit& operator=(strlit&& other) noexcept = default;

        [[nodiscard]] _inline_always constexpr const T* data() const noexcept { return this->lit; }
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        [[nodiscard]] _inline_always constexpr operator const T*() const noexcept { return this->data(); }

        [[nodiscard]] _inline_always constexpr bool empty() const noexcept { return *this->lit == 0; }
        [[nodiscard]] _inline_always constexpr size_t size() const noexcept { return std::char_traits<T>::length(this->lit); }

        [[nodiscard]] _inline_always constexpr iter begin() const noexcept { return iter(this->lit); }
        [[nodiscard]] _inline_always constexpr iter end() const noexcept { return iter(); }
        [[nodiscard]] _inline_always constexpr iter cbegin() const noexcept { return this->begin(); }
        [[nodiscard]] _inline_always constexpr iter cend() const noexcept { return this->end(); }
    };

    static_assert(sizeof(strlit<char>) == sizeof(const char*));
    static_assert(sizeof(strlit<wchar_t>) == sizeof(const wchar_t*));
    static_assert(sizeof(strlit<char8_t>) == sizeof(const char8_t*));
    static_assert(sizeof(strlit<char16_t>) == sizeof(const char16_t*));
    static_assert(sizeof(strlit<char32_t>) == sizeof(const char32_t*));

    using cstr_liter = strlit<char>;
    using wstr_liter = strlit<wchar_t>;
    using str_liter = strlit<char8_t>;
    using str16_liter = strlit<char16_t>;
    using str32_liter = strlit<char32_t>;
} // namespace sys
