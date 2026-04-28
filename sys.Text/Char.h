#pragma once

/// @file

#include <algorithm>
#include <concepts>
#include <span>
#include <string>
#include <string_view>

#include <Integer.h>
#include <LanguageSupport.h>
#include <data/UnicodeCasing.h>
#include <data/UnicodeWhitespace.h>
#include <meta/Builtin.h>
#include <meta/TypeSwitch.h>

namespace sys
{
    /// @ingroup sys_text
    /// @brief Character traits.
    class ch final
    {
    public:
        ch() = delete;

        /// @brief Unicode character type equivalent to `T`.
        template <ICharacter T>
        using unicode_equiv =
            meta::type_switch<meta::type_case<std::same_as<T, char32_t>, char32_t>, meta::type_case<std::same_as<T, char16_t>, char16_t>,
                              meta::type_case<std::same_as<T, char8_t>, char8_t>, meta::type_case<sizeof(T) == sizeof(char8_t), char8_t>,
                              meta::type_case<sizeof(T) == sizeof(char16_t), char16_t>, meta::type_case<sizeof(T) == sizeof(char32_t), char32_t>, meta::type_case<true, void>>;

        /// @brief The replacement character, U+FFFD.
        template <ICharacter T>
        static consteval std::basic_string_view<T> replacement() noexcept
        {
            if constexpr (std::same_as<T, char8_t>)
                return u8"\uFFFD";
            else if constexpr (std::same_as<T, char16_t>)
                return u"\uFFFD";
            else if constexpr (std::same_as<T, char32_t>)
                return U"\uFFFD";
            else if constexpr (std::same_as<T, char>)
                return "\uFFFD";
            else if constexpr (std::same_as<T, wchar_t>)
                return L"\uFFFD";
        }

        /// @brief Checks if a codepoint is a UTF-8 continuation byte.
        template <ICharacter T>
        static constexpr bool is_continuation(const T b) noexcept
        {
            return (u32(_as(b, u32::underlying_type)) & 0xC0_u32) == 0x80_u32; // NOLINT(readability-magic-numbers)
        }
        /// @brief Checks if a codepoint is a UTF-16 surrogate.
        template <ICharacter T>
        static constexpr bool is_surrogate(const T c) noexcept
        {
            return _as(c, u32::underlying_type) >= 0xD800_u32 && _as(c, u32::underlying_type) <= 0xDFFF_u32; // NOLINT(readability-magic-numbers)
        }
        /// @brief Checks if a codepoint is a high UTF-16 surrogate.
        template <ICharacter T>
        static constexpr bool is_high_surrogate(const T c) noexcept
        {
            return _as(c, u32::underlying_type) >= 0xD800_u32 && _as(c, u32::underlying_type) <= 0xDBFF_u32; // NOLINT(readability-magic-numbers)
        }
        /// @brief Checks if a codepoint is a low UTF-16 surrogate.
        template <ICharacter T>
        static constexpr bool is_low_surrogate(const T c) noexcept
        {
            return _as(c, u32::underlying_type) >= 0xDC00_u32 && _as(c, u32::underlying_type) <= 0xDFFF_u32; // NOLINT(readability-magic-numbers)
        }
        /// @brief Checks if a codepoint is a valid scalar value.
        template <ICharacter T>
        static constexpr bool is_scalar(const T cp) noexcept
        {
            return _as(cp, u32::underlying_type) <= 0x10FFFF_u32 && !ch::is_surrogate(cp); // NOLINT(readability-magic-numbers)
        }

        /// @brief Checks if a codepoint is considered whitespace.
        template <ICharacter T>
        static constexpr bool is_whitespace(const T c) noexcept
        {
            return internal::dchar_is_ws(_as(c, char32_t));
        }

        /// @brief Converts a codepoint to lowercase (simple mapping).
        template <ICharacter T>
        static constexpr char32_t to_lower(const T c) noexcept
        {
            return internal::dchar_to_lower_simple(_as(c, char32_t));
        }
        /// @brief Converts a codepoint to uppercase (simple mapping).
        template <ICharacter T>
        static constexpr char32_t to_upper(const T c) noexcept
        {
            return internal::dchar_to_upper_simple(_as(c, char32_t));
        }
        /// @brief Folds a codepoint (simple folding).
        template <ICharacter T>
        static constexpr char32_t fold(const T c) noexcept
        {
            return internal::dchar_fold_simple(_as(c, char32_t));
        }

        /// @brief The number of buffer elements in a null-terminated string.
        /// @warning `unsafe` because `cstr` has preconditions.
        /// @pre `const char cstr[N] && cstr != nullptr && N > 0uz && cstr[N - 1z] == '\0'`
        template <ICharacter T>
        static constexpr sz buffer_size(const T* cstr, decltype(unsafe)) noexcept(noexcept(std::char_traits<T>::length(cstr)))
        {
            return std::char_traits<T>::length(cstr);
        }

        struct codepoint_data
        {
            char32_t c;
            sz size_bytes;
        };
    private:
        static consteval codepoint_data read_codepoint_fail() noexcept
        {
            return codepoint_data { .c = ch::replacement<char32_t>()[0] /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) */, .size_bytes = 1_uz };
        }
    public:
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)

        /// @brief Reads a (possibly multibyte) codepoint from a unicode buffer.
        /// @return `(char32_t codepoint, sz elements_read)`
        /// @warning `unsafe` because `range` has preconditions.
        /// @pre `range.empty() == false`
        static constexpr codepoint_data read_codepoint(const std::span<const char32_t> range, decltype(unsafe)) noexcept
        {
            const u32 ret = _as(range[0] /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) */, u32::underlying_type);
            _retif(ch::read_codepoint_fail(), !ch::is_scalar(_as(*ret, char32_t)));
            return codepoint_data { .c = _as(*ret, char32_t), .size_bytes = 1_uz };
        }
        /// @see `sys::ch::read_codepoint(const std::span<const char32_t>, decltype(unsafe))`
        static constexpr codepoint_data read_codepoint(const std::span<const char16_t> range, decltype(unsafe)) noexcept
        {
            const u32 lead = _as(range[0] /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) */, u32::underlying_type);
            _retif((codepoint_data { .c = _as(*lead, char32_t), .size_bytes = 1_uz }), !ch::is_surrogate(_as(*lead, char32_t))); // BMP
            _retif(ch::read_codepoint_fail(), !ch::is_high_surrogate(_as(*lead, char32_t)) || range.size() < 2uz);

            const u32 trail = _as(range[1] /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) */, u32::underlying_type);
            _retif(ch::read_codepoint_fail(), !ch::is_low_surrogate(_as(*trail, char32_t)));

            return codepoint_data { .c = _as(*(0x10000_u32 + ((lead - 0xD800_u32) << 10_u32) + (trail - 0xDC00_u32)), char32_t), .size_bytes = 2_uz };
        }
        /// @see `sys::ch::read_codepoint(const std::span<const char32_t>, decltype(unsafe))`
        static constexpr codepoint_data read_codepoint(const std::span<const char8_t> range, decltype(unsafe)) noexcept // NOLINT(readability-function-cognitive-complexity)
        {
            u32 ret = _as(range[0] /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) */, u32::underlying_type);
            _retif((codepoint_data { .c = _as(*ret, char32_t), .size_bytes = 1_uz }), ret < 0x80_u32); // 1-byte sequence, fast path.

            sz len = 0_uz;
            if (ret >= 0xC2_u32 && ret < 0xE0_u32)
            {
                ret &= 0x1F_u32;
                len = 2_uz;
            }
            else if (ret < 0xF0_u32)
            {
                ret &= 0x0F_u32;
                len = 3_uz;
            }
            else if (ret < 0xF5_u32)
            {
                ret &= 0x07_u32;
                len = 4_uz;
            }
            else
                return ch::read_codepoint_fail();
            _retif(ch::read_codepoint_fail(), range.size() < len);

            for (sz i = 1_uz; i < len; i++)
            {
                const u32 bi = _as(range[i] /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) */, u32::underlying_type);
                _retif(ch::read_codepoint_fail(), !ch::is_continuation(_as(*bi, char8_t)));
                ret = (ret << 6_u32) | (bi & 0x3F_u32);
            }

            // Overlong + Scalar Checks
            constexpr u32 cpMinReq[] = { 0_u32, 0_u32, 0x80_u32, 0x800_u32, 0x10000_u32 };
            _retif(ch::read_codepoint_fail(), ret < cpMinReq[len] /* NOLINT(cppcoreguidelines-pro-bounds-constant-array-index) */ || !ch::is_scalar(_as(*ret, char32_t)));

            return codepoint_data { .c = _as(*ret, char32_t), .size_bytes = len };
        }
        /// @see `sys::ch::read_codepoint(const std::span<const char32_t>, decltype(unsafe))`
        static constexpr codepoint_data read_codepoint(const std::span<const char> range, decltype(unsafe)) noexcept
        {
            using ctype = ch::unicode_equiv<char>;
            return read_codepoint(std::span<const ctype>(_asr(range.data(), const ctype*), range.size()), unsafe);
        }
        /// @see `sys::ch::read_codepoint(const std::span<const char32_t>, decltype(unsafe))`
        static constexpr codepoint_data read_codepoint(const std::span<const wchar_t> range, decltype(unsafe)) noexcept
        {
            using ctype = ch::unicode_equiv<wchar_t>;
            return read_codepoint(std::span<const ctype>(_asr(range.data(), const ctype*), range.size()), unsafe);
        }

        /// @brief Writes a codepoint to a buffer.
        /// @param c Codepoint value.
        /// @param out Buffer to write to, which must be at least `sizeof(char32_t)` bytes (2 `char16_t`s or 4 `char8_t`s).
        /// @return The number of elements written.
        /// @warning `unsafe` because `out` has preconditions.
        /// @pre `sys::ICharacter out[N] && sizeof(out) >= sizeof(char32_t)`
        static constexpr sz write_codepoint(char32_t c, char32_t out[], decltype(unsafe)) noexcept
        {
            const u32 ret = _as(c, u32::underlying_type);
            out[0] = ch::is_scalar(_as(*ret, char32_t)) ? _as(*ret, char32_t)
                                                        : ch::replacement<char32_t>()[0] /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) */;
            return 1_uz;
        }
        /// @see `sys::ch::write_codepoint(char32_t, char32_t[], decltype(unsafe))`
        static constexpr sz write_codepoint(char32_t c, char16_t out[], decltype(unsafe)) noexcept
        {
            const u32 ret = _as(c, u32::underlying_type);
            if (!ch::is_scalar(_as(*ret, char32_t))) [[unlikely]]
            {
                std::ranges::copy(ch::replacement<char16_t>(), out);
                return ch::replacement<char16_t>().size();
            }

            if (ret <= 0xFFFF_u32)
            {
                out[0] = _as(*ret, char16_t);
                return 1_uz;
            }

            const u32 val = ret - 0x10000_u32;
            out[0] = _as(*(0xD800_u32 + (val >> 10_u32)), char16_t);
            out[1] = _as(*(0xDC00_u32 + (val & 0x3FF_u32)), char16_t);
            return 2_uz;
        }
        /// @see `sys::ch::write_codepoint(char32_t, char32_t[], decltype(unsafe))`
        static constexpr sz write_codepoint(char32_t c, char8_t out[], decltype(unsafe)) noexcept // NOLINT(readability-function-cognitive-complexity)
        {
            const u32 ret = _as(c, u32::underlying_type);
            if (ch::is_scalar(_as(*ret, char32_t))) [[likely]]
            {
                if (ret <= 0x7F_u32)
                {
                    out[0] = _as(*ret, char8_t);
                    return 1_uz;
                }
                if (ret <= 0x7FF_u32)
                {
                    out[0] = _as(*(0xC0_u32 | (ret >> 6_u32)), char8_t);
                    out[1] = _as(*(0x80_u32 | (ret & 0x3F_u32)), char8_t);
                    return 2_uz;
                }
                if (ret <= 0xFFFF_u32)
                {
                    out[0] = _as(*(0xE0_u32 | (ret >> 12_u32)), char8_t);
                    out[1] = _as(*(0x80_u32 | ((ret >> 6_u32) & 0x3F_u32)), char8_t);
                    out[2] = _as(*(0x80_u32 | (ret & 0x3F_u32)), char8_t);
                    return 3_uz;
                }
                if (ret <= 0x10FFFF_u32)
                {
                    out[0] = _as(*(0xF0_u32 | (ret >> 18_u32)), char8_t);
                    out[1] = _as(*(0x80_u32 | ((ret >> 12_u32) & 0x3F_u32)), char8_t);
                    out[2] = _as(*(0x80_u32 | ((ret >> 6_u32) & 0x3F_u32)), char8_t);
                    out[3] = _as(*(0x80_u32 | (ret & 0x3F_u32)), char8_t);
                    return 4_uz;
                }
            }

            std::ranges::copy(ch::replacement<char8_t>(), out);
            return ch::replacement<char8_t>().size();
        }
        /// @see `sys::ch::write_codepoint(char32_t, char32_t[], decltype(unsafe))`
        static constexpr sz write_codepoint(char32_t c, char out[], decltype(unsafe)) noexcept
        {
            using ctype = ch::unicode_equiv<char>;
            return write_codepoint(c, _asr(out, ctype*), unsafe);
        }
        /// @see `sys::ch::write_codepoint(char32_t, char32_t[], decltype(unsafe))`
        static constexpr sz write_codepoint(char32_t c, wchar_t out[], decltype(unsafe)) noexcept
        {
            using ctype = ch::unicode_equiv<wchar_t>;
            return write_codepoint(c, _asr(out, ctype*), unsafe);
        }

        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
    };
} // namespace sys
