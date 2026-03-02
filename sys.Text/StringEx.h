#pragma once

/// @file StringEx.h

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <Char.h>
#include <CodepointIterator.h>
#include <LanguageSupport.h>
#include <Numeric.h>
#include <Traits.h>
#include <data/UnicodeCCC.h>
#include <data/UnicodeCasing.h>

namespace sys
{
    template <ICharacter T>
    struct codepoint_view;

    /// @brief Unicode string container.
    template <ICharacter T>
    class string final
    {
        std::basic_string<T> str;

        // Internal: Casing utilities.

        static constexpr bool resets_combining(canonical_combining_class ccc) noexcept
        {
            return ccc == canonical_combining_class::not_reordered || ccc == canonical_combining_class::above;
        }
        static constexpr std::vector<lookahead_casing_context> lookahead_contexts_for_str(std::span<const char32_t> codepoints)
        {
            std::vector<lookahead_casing_context> ret(codepoints.size());

            bool followedByCased = false;
            bool beforeDot = false;
            char32_t nextCp = U'\0';
            for (ssz i = ssz(codepoints.size()) - 1_z; i >= 0_z; i--)
            {
                const char32_t c = codepoints[sz(i)];
                lookahead_casing_context& lctx = ret[sz(i)];
                lctx.followed_by_cased = followedByCased;
                lctx.more_above = nextCp != U'\0' && internal::dchar_ccc(nextCp) == canonical_combining_class::above;
                lctx.before_dot = beforeDot;

                if (internal::dchar_is_cased(c))
                    followedByCased = true;
                else if (!internal::dchar_is_case_ignorable(c))
                    followedByCased = false;

                if (c == U'\u0307')
                    beforeDot = true;
                else if (string::resets_combining(internal::dchar_ccc(c)))
                    beforeDot = false;

                nextCp = c;
            }

            return ret;
        }
        static constexpr void update_fcontext_for_char(forward_casing_context& ctx, const char32_t c) noexcept
        {
            if (internal::dchar_is_cased(c))
                ctx.is_preceded_by_cased = true;
            else if (!internal::dchar_is_case_ignorable(c))
                ctx.is_preceded_by_cased = false;

            if (internal::dchar_is_soft_dotted(c))
                ctx.after_soft_dotted = true;
            else if (const canonical_combining_class ccc = internal::dchar_ccc(c); string::resets_combining(ccc))
                ctx.after_soft_dotted = false;

            if (c == U'\u0049')
                ctx.after_i = true;
            else if (const canonical_combining_class ccc = internal::dchar_ccc(c); string::resets_combining(ccc))
                ctx.after_i = false;
        }

        constexpr auto first_non_ws_beg() const
        {
            const T* const endPtr = std::to_address(this->cend());
            const codepoint_iter<T> end(endPtr, endPtr);

            auto ret = codepoint_iter<T>(std::to_address(this->cbegin()), endPtr);
            while (ret < end && ch::is_whitespace(*ret))
                ++ret;

            return this->cbegin() + (std::to_address(ret) - std::to_address(this->cbegin()));
        }
        constexpr auto last_ws_beg() const
        {
            const T* const endPtr = std::to_address(this->cend());
            const codepoint_iter<T> end(endPtr, endPtr);

            const T* ret = std::to_address(this->cbegin());
            auto it = codepoint_iter<T>(std::to_address(this->cbegin()), endPtr);
            while (it < end)
            {
                if (!ch::is_whitespace(*it))
                    ret = std::to_address(++it);
                else
                    ++it;
            }

            return this->cbegin() + (ret - std::to_address(this->cbegin()));
        }
        template <bool IsUpper>
        constexpr string as_cased(std::u8string_view lang) const
        {
            string ret;
            ret.reserve(this->capacity());

            std::vector<char32_t> codepoints;
            codepoints.reserve(this->size());
            for (const char32_t c : codepoint_view<T>(*this))
                codepoints.push_back(c);
            _retif(ret, codepoints.empty());

            std::vector<lookahead_casing_context> lookaheads = string::lookahead_contexts_for_str(codepoints);
            forward_casing_context fctx;
            for (sz i = 0_uz; i < sz(codepoints.size()); i++)
            {
                const char32_t c = codepoints[i];
                const lookahead_casing_context& lctx = lookaheads[i];

                char32_t conv[3];
                sz convSize = 0_uz;
                if constexpr (IsUpper)
                    convSize = internal::dchar_to_upper_special(conv /* NOLINT(hicpp-no-array-decay) */, c, lang, fctx, lctx, unsafe());
                else
                    convSize = internal::dchar_to_lower_special(conv /* NOLINT(hicpp-no-array-decay) */, c, lang, fctx, lctx, unsafe());

                T buf[sizeof(char32_t) / sizeof(T)];
                for (sz j = 0_uz; j < convSize; j++)
                    ret.append(std::span(buf, ch::write_codepoint(conv[j], buf, unsafe()))); /* NOLINT(cppcoreguidelines-pro-bounds-constant-array-index) */

                string::update_fcontext_for_char(fctx, c);
            }

            return ret;
        }
    public:
        /// @brief Empty string.
        constexpr string() = default;
        constexpr string(std::nullptr_t) = delete;
        template <size_t N>
        requires (N > 0uz)
        constexpr /* NOLINT(hicpp-explicit-conversions) */ string(const T (&str)[N]) : str(str, str + N - 1z)
        { }
        constexpr string(const std::initializer_list<T> il) : str(il) { }
        /// @warning `unsafe` because `cstr` has preconditions.
        /// @pre `const T cstr[N]` => `cstr != nullptr && cstr[N - 1z] == '\0'`
        constexpr string(const T* cstr, unsafe) : string(cstr, cstr + ch::buffer_size(cstr, unsafe())) { }
        constexpr /* NOLINT(hicpp-explicit-conversions) */ string(std::basic_string<T> str) : str(std::move(str)) { }
        constexpr explicit string(const std::span<const T> data) : string(data.begin(), data.end()) { }
        constexpr explicit string(const std::basic_string_view<T> data) : string(data.begin(), data.end()) { }
        /// @pre `++...beg == end`
        template <std::input_iterator It>
        constexpr explicit string(It beg, It end) : str(beg, end)
        { }
        template <IEnumerable Container>
        constexpr explicit string(const Container& container) : string(std::begin(container), std::end(container))
        { }
        /// @brief Construct from a single character.
        constexpr /* NOLINT(hicpp-explicit-conversions) */ string(const T c) : str(1uz, c) { }
        /// @brief Repeat a character.
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
        [[nodiscard]] constexpr /* NOLINT(hicpp-explicit-conversions) */ operator std::span<const T>() const { return std::span<const T>(this->data(), this->size()); }
        [[nodiscard]] constexpr /* NOLINT(hicpp-explicit-conversions) */ operator std::span<T>() { return std::span<T>(this->data(), this->size()); }
        [[nodiscard]] constexpr /* NOLINT(hicpp-explicit-conversions) */ operator std::basic_string_view<T>() const
        {
            return std::basic_string_view<T>(this->data(), this->size());
        }
        /// @warning `unsafe` because `i` has preconditions.
        /// @pre `i < this->size()`
        constexpr T& operator[](const sz i, unsafe) { return this->str[i]; }
        /// @overload
        constexpr const T& operator[](const sz i, unsafe) const { return this->str[i]; }

        /// @brief Transcode between unicode strings of different character types.
        template <ICharacter U>
        requires (!std::same_as<T, U>)
        constexpr explicit string(const std::basic_string_view<U> other)
        {
            for (const char32_t c : codepoint_view(std::span(other)))
            {
                T buf[(sizeof(char32_t) / sizeof(T)) + 1uz] {};
                this->append(std::span(buf, ch::write_codepoint(c, buf, unsafe())));
            }
        }
        /// @see `sys::string<T>::string(const std::basic_string_view<U>)`
        template <ICharacter U>
        requires (!std::same_as<T, U>)
        constexpr explicit string(const std::basic_string<U>& other) : string(std::basic_string_view<U>(other))
        { }
        /// @see `sys::string<T>::string(const std::basic_string_view<U>)`
        template <ICharacter U>
        requires (!std::same_as<T, U>)
        constexpr explicit string(const sys::string<U>& other) : string(_as(std::basic_string_view<U>, other))
        { }

        [[nodiscard]] bool empty() const { return this->str.empty(); }
        [[nodiscard]] sz size() const { return this->str.size(); }
        [[nodiscard]] sz capacity() const { return this->str.capacity(); }
        [[nodiscard]] const T* c_str() const { return this->str.data(); }
        [[nodiscard]] T* data() { return this->str.data(); }
        [[nodiscard]] const T* data() const { return this->str.data(); }

        [[nodiscard]] constexpr auto begin() const { return this->str.cbegin(); }
        [[nodiscard]] constexpr auto end() const { return this->str.cend(); }
        [[nodiscard]] constexpr auto cbegin() const { return this->str.cbegin(); }
        [[nodiscard]] constexpr auto cend() const { return this->str.cend(); }
        [[nodiscard]] constexpr auto rbegin() const { return this->str.crbegin(); }
        [[nodiscard]] constexpr auto rend() const { return this->str.crend(); }
        [[nodiscard]] constexpr auto crbegin() const { return this->str.crbegin(); }
        [[nodiscard]] constexpr auto crend() const { return this->str.crend(); }

        /// @warning `unsafe` because `this` has preconditions.
        /// @pre `!this->empty()`
        [[nodiscard]] constexpr T& front(unsafe) { return this->str.front(); }
        /// @overload
        [[nodiscard]] constexpr const T& front(unsafe) const { return this->str.front(); }
        /// @warning `unsafe` because `this` has preconditions.
        /// @pre `!this->empty()`
        [[nodiscard]] constexpr T& back(unsafe) { return this->str.back(); }
        /// @overload
        [[nodiscard]] constexpr const T& back(unsafe) const { return this->str.back(); }

        [[nodiscard]] bool contains(const std::basic_string_view<T> substr) const { return this->str.contains(substr); }
        [[nodiscard]] bool starts_with(const T c) const { return this->str.starts_with(c); }
        [[nodiscard]] bool starts_with(const std::basic_string_view<T> substr) const { return this->str.starts_with(substr); }
        [[nodiscard]] bool ends_with(const T c) const { return this->str.ends_with(c); }
        [[nodiscard]] bool ends_with(const std::basic_string_view<T> substr) const { return this->str.ends_with(substr); }

        /// @brief Find the index of the first occurrence of `c` from `from`.
        [[nodiscard]] sz find_index(const T c, const sz from = 0_uz) const
        {
            _retif(this->size(), from >= this->size());
            const sz ret = this->str.find(c, from);
            return ret != std::basic_string_view<T>::npos ? ret : this->size();
        }
        /// @brief Find the index of the first occurrence of `substr` from `from`.
        [[nodiscard]] sz find_index(const std::basic_string_view<T> substr, const sz from = 0_uz) const
        {
            _retif(this->size(), from >= this->size());
            const sz ret = this->str.find(substr, from);
            return ret != std::basic_string_view<T>::npos ? ret : this->size();
        }
        /// @brief Substring of range [`from`, `from` + `count`).
        [[nodiscard]] string substr(const sz from, const sz count) const
        {
            _retif({}, from >= this->size());
            return this->str.substr(from, count);
        }

        constexpr void reserve(const sz capacity) { this->str.reserve(capacity); }
        constexpr void clear() { this->str.clear(); }

        /// @brief Append `c` to the end of the string.
        constexpr string& append(const T c) &
        {
            this->str.push_back(c);
            return *this;
        }
        /// @overload
        constexpr string append(const T c) && { return this->append(c), std::move(*this); }
        /// @brief Append `c` to the end of the string.
        constexpr string& append(const T c, const sz count) &
        {
            this->str.append(count, c);
            return *this;
        }
        /// @overload
        constexpr string append(const T c, const sz count) && { return this->append(c, count), std::move(*this); }
        /// @brief Append `data` to the end of the string.
        constexpr string& append(const std::span<const T> data) &
        {
            this->str.append(data.data(), data.size());
            return *this;
        }
        /// @overload
        constexpr string append(const std::span<const T> data) && { return this->append(data), std::move(*this); }

        /// @brief Remove the last character from the string.
        /// @warning `unsafe` because `this` has preconditions.
        /// @pre `!this->empty()`
        constexpr string& pop_back(unsafe) &
        {
            this->str.pop_back();
            return *this;
        }
        /// @overload
        constexpr string pop_back(unsafe) && { return this->pop_back(unsafe()), std::move(*this); }
        /// @brief Erase the last character, if string non-empty.
        constexpr string& pop_back() &
        {
            if (!this->empty())
                this->pop_back(unsafe());
            return *this;
        }
        /// @overload
        constexpr string pop_back() && { return this->pop_back(), std::move(*this); }

        /// @brief Obtain a copy with leading and trailing whitespace removed.
        constexpr string trimmed() const
        {
            const auto from = this->first_non_ws_beg(), to = this->last_ws_beg();
            _retif({}, from >= to);
            return std::basic_string<T>(from, to);
        }
        /// @brief Remove leading and trailing whitespace.
        constexpr string& trim() &
        {
            this->trim_end();
            return this->trim_start();
        }
        /// @overload
        constexpr string trim() && { return this->trim(), std::move(*this); }
        /// @brief Obtain a copy with leading whitespace removed.
        constexpr string start_trimmed() const { return std::basic_string<T>(this->first_non_ws_beg(), this->cend()); }
        /// @brief Remove leading whitespace.
        constexpr string& trim_start() & { return this->str.erase(this->cbegin(), this->first_non_ws_beg()), *this; }
        /// @overload
        constexpr string trim_start() && { return this->trim_start(), std::move(*this); }
        /// @brief Obtain a copy with trailing whitespace removed.
        constexpr string end_trimmed() const { return std::basic_string<T>(this->cbegin(), this->last_ws_beg()); }
        /// @brief Remove trailing whitespace.
        constexpr string& trim_end() & { return this->str.erase(this->last_ws_beg(), this->cend()), *this; }
        /// @overload
        constexpr string trim_end() && { return this->trim_end(), std::move(*this); }

        /// @brief Obtain a copy with invalids replaced with U+FFFD.
        constexpr string invalids_replaced() const
        {
            string ret;
            ret.reserve(this->capacity());
            for (const char32_t c : codepoint_view(*this))
            {
                T buf[(sizeof(char32_t) / sizeof(T)) + 1uz] {};
                ret.append(std::span(buf, ch::write_codepoint(c, buf, unsafe())));
            }
            return ret;
        }
        /// @brief Replace invalid codepoints with U+FFFD.
        constexpr string& replace_invalid() & { return (*this = this->invalids_replaced()); }
        /// @overload
        constexpr string replace_invalid() && { return this->invalids_replaced(); }

        /// @brief Obtain a copy as lowercase.
        constexpr string lowered(std::u8string_view lang = u8"") const { return this->as_cased<false>(lang); }
        /// @brief Convert to lowercase.
        constexpr string& to_lower(std::u8string_view lang = u8"") & { return (*this = this->lowered(lang)); }
        /// @overload
        constexpr string to_lower(std::u8string_view lang = u8"") && { return this->lowered(lang); }
        /// @brief Obtain a copy as uppercase.
        constexpr string uppered(std::u8string_view lang = u8"") const { return this->as_cased<true>(lang); }
        /// @brief Convert to uppercase.
        constexpr string& to_upper(std::u8string_view lang = u8"") & { return (*this = this->uppered(lang)); }
        /// @overload
        constexpr string to_upper(std::u8string_view lang = u8"") && { return this->uppered(lang); }

        /// @brief Obtain a copy case folded.
        constexpr string folded(std::u8string_view lang = u8"") const
        {
            string ret;
            ret.reserve(this->capacity());
            for (const char32_t c : codepoint_view(*this))
            {
                char32_t conv[3];
                const sz convSize = internal::dchar_fold_special(conv /* NOLINT(hicpp-no-array-decay) */, c, lang, unsafe());

                T buf[sizeof(char32_t) / sizeof(T)];
                for (sz i = 0_uz; i < convSize; i++)
                    ret.append(std::span(buf, ch::write_codepoint(conv[i] /* NOLINT(cppcoreguidelines-pro-bounds-constant-array-index) */, buf, unsafe())));
            }
            return ret;
        }
        /// @brief Transform with canonical case folding.
        constexpr string& fold(std::u8string_view lang = u8"") & { return (*this = this->folded(lang)); }
        /// @overload
        constexpr string fold(std::u8string_view lang = u8"") && { return this->folded(lang); }

        /// @brief Split the string into substrings separated by `delimiter`.
        template <IAppendable<string> Container = std::vector<string>>
        [[nodiscard]] Container split(const T delimiter) const
        {
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
        /// @brief Split the string into substrings separated by `delimiter`.
        template <typename Container = std::vector<string>>
        requires IAppendable<Container, T> && IAppendable<Container, string>
        [[nodiscard]] Container split(const std::basic_string_view<T> delimiter) const
        {
            if (delimiter.empty())
            {
                Container ret;
                ret.reserve(this->size());
                for (const T c : *this)
                    meta::append_to(ret, c);
                return ret;
            }

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
        /// @brief Join the strings in `container` with `sep`.
        template <typename Container, typename Chars>
        requires IEnumerable<Container> && IEmptyQueryable<Container>
        [[nodiscard]] static string join(const Container& container, const Chars& sep)
        {
            if (meta::is_empty(container))
                return {};

            sz totalSize = 0_uz;
            for (const auto& s : container)
            {
                if constexpr (ICharacter<std::remove_cvref_t<decltype(s)>>)
                    totalSize += 1_uz;
                else
                    totalSize += std::size(s);
            }
            if (std::size(container) > 1)
            {
                if constexpr (ICharacter<std::remove_cvref_t<decltype(sep)>>)
                    totalSize += sz(std::size(container)) - 1_uz;
                else if constexpr (std::is_array_v<std::remove_cvref_t<decltype(sep)>>)
                    totalSize += (sz(std::size(sep)) - 1_uz) * (sz(std::size(container)) - 1_uz);
                else
                    totalSize += sz(std::size(sep)) * (sz(std::size(container)) - 1_uz);
            }

            string ret;
            ret.reserve(totalSize);

            bool needPrependSep = false;
            for (const auto& part : container)
            {
                if (needPrependSep)
                {
                    if constexpr (std::is_array_v<std::remove_cvref_t<decltype(sep)>>)
                        ret.append(std::basic_string_view(sep));
                    else
                        ret.append(sep);
                }

                if constexpr (requires { ret.append(part); })
                    ret.append(part);
                else
                    ret.append(_as(string, part));
                needPrependSep = true;
            }

            return ret;
        }

        friend void swap(string& a, string& b) noexcept
        {
            using std::swap;
            swap(a.str, b.str);
        }

        template <ICharacter U>
        friend class string;
    };

    template <ICharacter T>
    string(std::basic_string_view<T>) -> string<T>;
    template <ICharacter T>
    string(std::basic_string<T>) -> string<T>;

    using cstr = string<char>;
    using wstr = string<wchar_t>;
    using str = string<char8_t>;
    using str16 = string<char16_t>;
    using str32 = string<char32_t>;
} // namespace sys

/// @brief `std::formatter<...>` specialization for `sys::string<...>`.
template <sys::ICharacter T, sys::ICharacter FormatChar>
struct std::formatter<sys::string<T>, FormatChar> : std::formatter<std::basic_string_view<FormatChar>, FormatChar>
{
    /// @brief Formats a `sys::string<T>` as a `std::basic_string_view<FormatChar>`.
    template <typename FormatContext>
    auto format(const sys::string<T>& str, FormatContext& context) const
    {
        if constexpr (std::same_as<T, FormatChar>)
            return std::formatter<std::basic_string_view<FormatChar>, FormatChar>::format(std::basic_string_view<T>(str), context);
        else
            return std::formatter<std::basic_string_view<FormatChar>, FormatChar>::format(_as(std::basic_string_view<FormatChar>, sys::string<FormatChar>(str)), context);
    }
};
