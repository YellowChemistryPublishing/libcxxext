#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <Char.h>
#include <CodepointIterator.h>
#include <LanguageSupport.h>
#include <Numeric.h>
#include <Traits.h>

namespace sys
{
    template <ICharacter T>
    class string final
    {
        std::basic_string<T> str;
    public:
        constexpr string() = default;
        constexpr string(std::nullptr_t) = delete;
        template <size_t N>
        constexpr /* NOLINT(hicpp-explicit-conversions) */ string(const T (&str)[N]) : str(str, str + N - 1z)
        { }
        constexpr string(const std::initializer_list<T> il) : str(il) { }
        constexpr string(const T* cstr, unsafe) : string(cstr, cstr + (cstr ? ch::buffer_size(cstr, unsafe()) : 0_uz)) { }
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr string(std::basic_string<T> str) : str(std::move(str)) { }
        template <std::input_iterator It>
        constexpr explicit string(It beg, It end) : str(beg, end)
        { }
        constexpr explicit string(const std::span<T> data) : string(data.begin(), data.end()) { }
        template <IEnumerable Container>
        constexpr explicit string(const Container& container) : string(container.begin(), container.end())
        { }
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr string(const T c) : str(1uz, c) { }
        constexpr string(const T c, const sz count) : str(count, c) { }
        constexpr string(const string&) = default;
        constexpr string(string&& other) noexcept { swap(*this, other); }
        ~string() = default;

        constexpr string& operator=(const string&) = default;
        constexpr string& operator=(string&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        friend constexpr bool operator==(const string&, const string&) = default;
        template <size_t N>
        friend constexpr bool operator==(const string& a, const T (&b)[N])
        {
            return a.str == b;
        }
        template <size_t N>
        friend constexpr bool operator==(const T (&a)[N], const string& b)
        {
            return b.str == a;
        }
        friend constexpr bool operator==(const string& a, const std::basic_string_view<T>& b) { return a.str == b; }
        friend constexpr bool operator==(const std::basic_string_view<T>& a, const string& b) { return b.str == a; }
        friend constexpr auto operator<=>(const string&, const string&) = default;
        template <size_t N>
        friend constexpr auto operator<=>(const string& a, const T (&b)[N])
        {
            return a.str <=> b;
        }
        template <size_t N>
        friend constexpr auto operator<=>(const T (&a)[N], const string& b)
        {
            return b.str <=> a;
        }
        friend constexpr auto operator<=>(const string& a, const std::basic_string_view<T>& b) { return a.str <=> b; }
        friend constexpr auto operator<=>(const std::basic_string_view<T>& a, const string& b) { return b.str <=> a; }

        [[nodiscard]] constexpr explicit operator const T*() const { return this->str.data(); }
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        [[nodiscard]] constexpr operator std::basic_string_view<T>() const { return std::basic_string_view<T>(this->data(), this->size()); }
        constexpr T& operator[](const sz i) { return this->str[i]; }
        constexpr const T& operator[](const sz i) const { return this->str[i]; }

        /// @brief Convert between strings of different character types.
        template <ICharacter U>
        requires (!std::same_as<T, U>)
        constexpr explicit string(const string<U>& other)
        {
            this->reserve(other.size());
            for (const char32_t c : str32_view(other))
            {
                T buf[(sizeof(char32_t) / sizeof(T)) + 1uz] {};
                this->append(std::span(buf, ch::write_codepoint(c, buf, unsafe())));
            }
        }

        [[nodiscard]] bool empty() const { return this->str.empty(); }
        [[nodiscard]] sz size() const { return this->str.size(); }
        [[nodiscard]] const T* c_str() const { return this->str.data(); }
        [[nodiscard]] T* data() { return this->str.data(); }
        [[nodiscard]] const T* data() const { return this->str.data(); }

        [[nodiscard]] constexpr auto begin() { return this->str.begin(); }
        [[nodiscard]] constexpr auto end() { return this->str.end(); }
        [[nodiscard]] constexpr auto begin() const { return this->str.begin(); }
        [[nodiscard]] constexpr auto end() const { return this->str.end(); }
        [[nodiscard]] constexpr auto rbegin() { return this->str.rbegin(); }
        [[nodiscard]] constexpr auto rend() { return this->str.rend(); }
        [[nodiscard]] constexpr auto rbegin() const { return this->str.rbegin(); }
        [[nodiscard]] constexpr auto rend() const { return this->str.rend(); }
        [[nodiscard]] constexpr auto cbegin() const { return this->str.cbegin(); }
        [[nodiscard]] constexpr auto cend() const { return this->str.cend(); }
        [[nodiscard]] constexpr auto crbegin() const { return this->str.crbegin(); }
        [[nodiscard]] constexpr auto crend() const { return this->str.crend(); }

        [[nodiscard]] bool contains(const std::basic_string_view<T> substr) const { return this->str.contains(substr); }
        [[nodiscard]] bool starts_with(const std::basic_string_view<T> substr) const { return this->str.starts_with(substr); }
        [[nodiscard]] bool ends_with(const std::basic_string_view<T> substr) const { return this->str.ends_with(substr); }

        constexpr void reserve(const sz capacity) { this->str.reserve(capacity); }

        constexpr string& append(const T c)
        {
            this->str.push_back(c);
            return *this;
        }
        constexpr string& append(const T c, const sz count)
        {
            this->str.append(count, c);
            return *this;
        }
        constexpr string& append(const std::basic_string_view<T> str)
        {
            this->str.append(str);
            return *this;
        }
        constexpr string& append(const std::span<const T> data)
        {
            this->str.append(data.data(), data.size());
            return *this;
        }

        constexpr string& replace_invalid()
        {
            std::basic_string<T> normalized;
            normalized.reserve(this->size());
            for (const char32_t c : str32_view(*this))
            {
                T buf[(sizeof(char32_t) / sizeof(T)) + 1uz] {};
                normalized.append(std::basic_string_view<T>(buf, ch::write_codepoint(c, buf, unsafe())));
            }
            this->str = std::move(normalized);
            return *this;
        }

        constexpr string& trim()
        {
            this->trim_end();
            return this->trim_start();
        }
        constexpr string& trim_start()
        {
            const T* const endPtr = std::to_address(this->cend());
            const str32_iter<T> end(endPtr, endPtr);

            auto it = str32_iter<T>(std::to_address(this->cbegin()), endPtr);
            while (it < end && ch::is_whitespace(*it))
                ++it;

            if (std::to_address(it) > std::to_address(this->cbegin()))
                this->str.erase(this->cbegin(), this->cbegin() + (std::to_address(it) - std::to_address(this->cbegin())));

            return *this;
        }
        constexpr string& trim_end()
        {
            const T* const endPtr = std::to_address(this->cend());
            const str32_iter<T> end(endPtr, endPtr);

            const T* lastSpaceBeg = std::to_address(this->cbegin());
            auto it = str32_iter<T>(std::to_address(this->cbegin()), endPtr);
            while (it < end)
            {
                if (!ch::is_whitespace(*it))
                    lastSpaceBeg = std::to_address(++it);
                else
                    ++it;
            }

            if (lastSpaceBeg < std::to_address(this->cend()))
                this->str.erase(this->cbegin() + (lastSpaceBeg - std::to_address(this->cbegin())), this->cend());

            return *this;
        }

        template <IAppendable Container = std::vector<string>>
        [[nodiscard]] Container split(const T delimiter) const
        {
            if (this->empty())
                return { *this };

            Container ret;
            auto from = this->begin();
            for (auto it = this->begin(); it < this->end(); ++it)
            {
                if (*it == delimiter)
                {
                    meta::append_to(ret, string(from, it));
                    from = it + 1z;
                }
            }
            meta::append_to(ret, string(from, this->end()));

            return ret;
        }
        template <IAppendable Container = std::vector<string>>
        [[nodiscard]] Container split(const std::basic_string_view<T> delimiter) const
        {
            if (this->size() < delimiter.size())
                return { *this };

            Container ret;
            auto from = this->begin();
            for (auto it = this->begin(); it <= this->end() - delimiter.size(); ++it)
            {
                if (std::basic_string_view<T>(it, it + delimiter.size()) == delimiter)
                {
                    meta::append_to(ret, string(from, it));
                    it += delimiter.size() - 1z;
                    from = it + 1z;
                }
            }
            meta::append_to(ret, string(from, this->end()));

            return ret;
        }
        template <IAppendable Container = std::vector<string>>
        [[nodiscard]] static string join(const Container& container, const std::basic_string_view<T> sep)
        {
            if (container.empty())
                return {};

            size_t totalSize = 0;
            for (const string& s : container)
                totalSize += s.size();
            if (container.size() > 1)
                totalSize += sz(sep.size()) * (sz(container.size()) - 1_uz);

            std::basic_string<T> ret;
            ret.reserve(totalSize + 1);

            bool needPrependSep = false;
            for (const string& part : container)
            {
                if (needPrependSep)
                    ret.append(sep.begin(), sep.end());
                ret.append(part.begin(), part.end());
                needPrependSep = true;
            }

            return string(std::move(ret));
        }

        friend void swap(string& a, string& b) noexcept
        {
            using std::swap;
            swap(a.str, b.str);
        }

        template <ICharacter U>
        friend class string;
    };

    template <IUnicodeCharacter T>
    str32_view(const string<T>&) -> str32_view<T>;

    using cstr = string<char>;
    using wstr = string<wchar_t>;
    using str = string<char8_t>;
    using str16 = string<char16_t>;
    using str32 = string<char32_t>;
} // namespace sys
