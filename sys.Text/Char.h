#pragma once

#include <algorithm>
#include <concepts>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include <LanguageSupport.h>
#include <Numeric.h>
#include <Traits.h>
#include <data/UnicodeWhitespace.h>

namespace sys
{
    /// @brief Character traits.
    /// @note Static class.
    class ch
    {
    public:
        ch() = delete;

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
            return (u32(_as(u32::underlying_type, b)) & 0xC0_u32) == 0x80_u32; // NOLINT(readability-magic-numbers)
        }
        /// @brief Checks if a codepoint is a UTF-16 surrogate.
        template <ICharacter T>
        static constexpr bool is_surrogate(const T c) noexcept
        {
            return _as(u32::underlying_type, c) >= 0xD800_u32 && _as(u32::underlying_type, c) <= 0xDFFF_u32; // NOLINT(readability-magic-numbers)
        }
        /// @brief Checks if a codepoint is a high UTF-16 surrogate.
        template <ICharacter T>
        static constexpr bool is_high_surrogate(const T c) noexcept
        {
            return _as(u32::underlying_type, c) >= 0xD800_u32 && _as(u32::underlying_type, c) <= 0xDBFF_u32; // NOLINT(readability-magic-numbers)
        }
        /// @brief Checks if a codepoint is a low UTF-16 surrogate.
        template <ICharacter T>
        static constexpr bool is_low_surrogate(const T c) noexcept
        {
            return _as(u32::underlying_type, c) >= 0xDC00_u32 && _as(u32::underlying_type, c) <= 0xDFFF_u32; // NOLINT(readability-magic-numbers)
        }
        /// @brief Checks if a codepoint is a valid scalar value.
        template <ICharacter T>
        static constexpr bool is_scalar(const T cp) noexcept
        {
            return _as(u32::underlying_type, cp) <= 0x10FFFF_u32 && !ch::is_surrogate(cp); // NOLINT(readability-magic-numbers)
        }

        /// @brief Checks if a codepoint is considered whitespace.
        template <ICharacter T>
        static constexpr bool is_whitespace(const T c) noexcept
        {
            return internal::dchar_is_ws(_as(char32_t, c));
        }

        /// @brief The number of buffer elements in a null-terminated string.
        /// @warning `cstr` needs to be non-null and null-terminated.
        template <ICharacter T>
        static constexpr sz buffer_size(const T* cstr, unsafe) noexcept(noexcept(std::char_traits<T>::length(cstr)))
        {
            return std::char_traits<T>::length(cstr);
        }
    private:
        static consteval std::pair<char32_t, sz> read_codepoint_fail() noexcept { return { ch::replacement<char32_t>()[0], 1_uz }; }
    public:
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
        /// @brief Reads a (possibly multibyte) codepoint from a unicode buffer.
        /// @return `(char32_t codepoint, sz elements_read)`
        /// @note `unsafe` because `range` must not be empty.
        static constexpr std::pair<char32_t, sz> read_codepoint(const std::span<const char32_t> range, unsafe) noexcept
        {
            const u32 ret = _as(u32::underlying_type, range[0]);
            _retif(ch::read_codepoint_fail(), !ch::is_scalar(_as(char32_t, ret)));
            return { _as(char32_t, ret), 1_uz };
        }
        /// @see `sys::ch::read_codepoint(const char32_t* buf, const char32_t* const, unsafe)`
        static constexpr std::pair<char32_t, sz> read_codepoint(const std::span<const char16_t> range, unsafe) noexcept
        {
            const u32 lead = _as(u32::underlying_type, range[0]);
            _retif(std::make_pair(_as(char32_t, lead), 1_uz), !ch::is_surrogate(_as(char32_t, lead))); // BMP
            _retif(ch::read_codepoint_fail(), !ch::is_high_surrogate(_as(char32_t, lead)) || range.size() < 2uz);

            const u32 trail = _as(u32::underlying_type, range[1]);
            _retif(ch::read_codepoint_fail(), !ch::is_low_surrogate(_as(char32_t, trail)));

            return { _as(char32_t, 0x10000_u32 + ((lead - 0xD800_u32) << 10_u32) + (trail - 0xDC00_u32)), 2_uz };
        }
        /// @see `sys::ch::read_codepoint(const char32_t* buf, const char32_t* const, unsafe)`
        static constexpr std::pair<char32_t, sz> read_codepoint(const std::span<const char8_t> range, unsafe) noexcept // NOLINT(readability-function-cognitive-complexity)
        {
            u32 ret = _as(u32::underlying_type, range[0]);
            _retif(std::make_pair(_as(char32_t, ret), 1_uz), ret < 0x80_u32); // 1-byte sequence, fast path.

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
                const u32 bi = _as(u32::underlying_type, range[i]);
                _retif(ch::read_codepoint_fail(), !ch::is_continuation(_as(char8_t, bi)));
                ret = (ret << 6_u32) | (bi & 0x3F_u32);
            }

            // Overlong + Scalar Checks
            constexpr u32 cpMinReq[] = { 0_u32, 0_u32, 0x80_u32, 0x800_u32, 0x10000_u32 };
            _retif(ch::read_codepoint_fail(), ret < cpMinReq[len] || !ch::is_scalar(_as(char32_t, ret))); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)

            return { _as(char32_t, ret), len };
        }

        /// @brief Writes a codepoint to a buffer.
        /// @param out Buffer to write to, which must be at least `sizeof(char32_t)` bytes (2 `char16_t`s or 4 `char8_t`s).
        /// @return The number of elements written.
        /// @note `unsafe` because the byte-size of `out` must be `>= sizeof(char32_t)`.
        static constexpr sz write_codepoint(char32_t c, char32_t out[], unsafe) noexcept
        {
            const u32 ret = _as(u32::underlying_type, c);
            out[0] = ch::is_scalar(_as(char32_t, ret)) ? _as(char32_t, ret) : ch::replacement<char32_t>()[0];
            return 1_uz;
        }
        /// @see `sys::ch::write_codepoint(char32_t c, char32_t out[], unsafe)`
        static constexpr sz write_codepoint(char32_t c, char16_t out[], unsafe) noexcept
        {
            const u32 ret = _as(u32::underlying_type, c);
            if (!ch::is_scalar(_as(char32_t, ret))) [[unlikely]]
            {
                std::ranges::copy(ch::replacement<char16_t>(), out);
                return ch::replacement<char16_t>().size();
            }

            if (ret <= 0xFFFF_u32)
            {
                out[0] = _as(char16_t, ret);
                return 1_uz;
            }

            const u32 val = ret - 0x10000_u32;
            out[0] = _as(char16_t, 0xD800_u32 + (val >> 10_u32));
            out[1] = _as(char16_t, 0xDC00_u32 + (val & 0x3FF_u32));
            return 2_uz;
        }
        /// @see `sys::ch::write_codepoint(char32_t c, char32_t out[], unsafe)`
        static constexpr sz write_codepoint(char32_t c, char8_t out[], unsafe) noexcept // NOLINT(readability-function-cognitive-complexity)
        {
            const u32 ret = _as(u32::underlying_type, c);
            if (ch::is_scalar(_as(char32_t, ret))) [[likely]]
            {
                if (ret <= 0x7F_u32)
                {
                    out[0] = _as(char8_t, ret);
                    return 1_uz;
                }
                if (ret <= 0x7FF_u32)
                {
                    out[0] = _as(char8_t, 0xC0_u32 | (ret >> 6_u32));
                    out[1] = _as(char8_t, 0x80_u32 | (ret & 0x3F_u32));
                    return 2_uz;
                }
                if (ret <= 0xFFFF_u32)
                {
                    out[0] = _as(char8_t, 0xE0_u32 | (ret >> 12_u32));
                    out[1] = _as(char8_t, 0x80_u32 | ((ret >> 6_u32) & 0x3F_u32));
                    out[2] = _as(char8_t, 0x80_u32 | (ret & 0x3F_u32));
                    return 3_uz;
                }
                if (ret <= 0x10FFFF_u32)
                {
                    out[0] = _as(char8_t, 0xF0_u32 | (ret >> 18_u32));
                    out[1] = _as(char8_t, 0x80_u32 | ((ret >> 12_u32) & 0x3F_u32));
                    out[2] = _as(char8_t, 0x80_u32 | ((ret >> 6_u32) & 0x3F_u32));
                    out[3] = _as(char8_t, 0x80_u32 | (ret & 0x3F_u32));
                    return 4_uz;
                }
            }

            std::ranges::copy(ch::replacement<char8_t>(), out);
            return ch::replacement<char8_t>().size();
        }
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
    };
} // namespace sys
