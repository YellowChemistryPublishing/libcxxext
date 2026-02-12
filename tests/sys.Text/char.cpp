// NOLINTBEGIN(misc-include-cleaner, readability-magic-numbers, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay)

#include <catch2/catch_all.hpp>

#include <module/sys.Text>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::ch` Whitespace Verification", "[sys.Text][char][whitespace]")
{
    SECTION("Standard Whitespace")
    {
        CHECK(sys::ch::is_whitespace(' '));
        CHECK(sys::ch::is_whitespace('\t'));
        CHECK(sys::ch::is_whitespace('\n'));
        CHECK(sys::ch::is_whitespace('\r'));
        CHECK(sys::ch::is_whitespace('\f'));
        CHECK(sys::ch::is_whitespace('\v'));
    }

    SECTION("Unicode Whitespace (Exhaustive)")
    {
        // Space Separators
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x00A0))); // NBSP
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x1680))); // OGHAM SPACE MARK
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2000))); // EN QUAD
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2001))); // EM QUAD
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2002))); // EN SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2003))); // EM SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2004))); // THREE-PER-EM SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2005))); // FOUR-PER-EM SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2006))); // SIX-PER-EM SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2007))); // FIGURE SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2008))); // PUNCTUATION SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2009))); // THIN SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x200A))); // HAIR SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x202F))); // NARROW NO-BREAK SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x205F))); // MEDIUM MATHEMATICAL SPACE
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x3000))); // IDEOGRAPHIC SPACE

        // Line / Paragraph Separators
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2028))); // LINE SEPARATOR
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x2029))); // PARAGRAPH SEPARATOR

        // Other Control Characters
        CHECK(sys::ch::is_whitespace(_as(char32_t, 0x0085))); // NEXT LINE (NEL)
    }

    SECTION("Non-Whitespace Boundaries")
    {
        CHECK_FALSE(sys::ch::is_whitespace('A'));
        CHECK_FALSE(sys::ch::is_whitespace('0'));
        CHECK_FALSE(sys::ch::is_whitespace(_as(char32_t, 0x0000))); // Null-terminator doesn't count.
        CHECK_FALSE(sys::ch::is_whitespace(_as(char32_t, 0x167F))); // Just before Ogham.
        CHECK_FALSE(sys::ch::is_whitespace(_as(char32_t, 0x1681))); // Just after Ogham.
        CHECK_FALSE(sys::ch::is_whitespace(_as(char32_t, 0x200B))); // ZERO WIDTH SPACE
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::ch::read_codepoint(...)` UTF-8 Exhaustive", "[sys.Text][char][utf8][decode]")
{
    SECTION("Valid Sequences")
    {
        // 1-byte.
        char8_t buf1[] { 0x41 }; // 'A'
        auto [cp1, size1] = sys::ch::read_codepoint(std::span(buf1), unsafe());
        CHECK(cp1 == U'A');
        CHECK(size1 == 1_uz);

        // 2-byte.
        char8_t buf2[] { 0xC2, 0xA2 }; // Cent sign. (U+00A2)
        auto [cp2, size2] = sys::ch::read_codepoint(std::span(buf2), unsafe());
        CHECK(cp2 == 0x00A2);
        CHECK(size2 == 2_uz);

        // 3-byte.
        char8_t buf3[] { 0xE2, 0x82, 0xAC }; // Euro sign. (U+20AC)
        auto [cp3, size3] = sys::ch::read_codepoint(std::span(buf3), unsafe());
        CHECK(cp3 == 0x20AC);
        CHECK(size3 == 3_uz);

        // 4-byte.
        char8_t buf4[] { 0xF0, 0x90, 0x8D, 0x88 }; // Gothic letter hwair. (U+10348)
        auto [cp4, size4] = sys::ch::read_codepoint(std::span(buf4), unsafe());
        CHECK(cp4 == U'\U00010348');
        CHECK(size4 == 4_uz);
    }

    SECTION("Overlong Encodings (Produces Replacement Character)")
    {
        // 'A' (U+0041) as 2-byte. | `11000001` `10000001` -> `C1` `81`
        char8_t buf1[] { 0xC1, 0x81 };
        auto [cp1, size1] = sys::ch::read_codepoint(std::span(buf1), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp1 == sys::ch::replacement<char32_t>()[0]);

        // '/' (U+002F) as 2-byte. | `11000000` `10101111` -> `C0` `AF` (Security risk!)
        char8_t buf2[] { 0xC0, 0xAF };
        auto [cp2, size2] = sys::ch::read_codepoint(std::span(buf2), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp2 == sys::ch::replacement<char32_t>()[0]);

        // Null (U+0000) as 2-byte. | `11000000` `10000000` -> `C0` `80`
        char8_t buf3[] { 0xC0, 0x80 };
        auto [cp3, size3] = sys::ch::read_codepoint(std::span(buf3), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp3 == sys::ch::replacement<char32_t>()[0]);
    }

    SECTION("Invalid Start Bytes")
    {
        for (char8_t b : { _as(char8_t, 0x80), _as(char8_t, 0xBF), _as(char8_t, 0xF5), _as(char8_t, 0xFF) })
        {
            auto [cp, size] = sys::ch::read_codepoint(std::span(&b, 1uz), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            CHECK(cp == sys::ch::replacement<char32_t>()[0]);
        }
    }

    SECTION("Mismatched / Truncated Sequences")
    {
        // Truncated 2-byte sequence.
        char8_t buf1[] { 0xC2 };
        auto [cp1, size1] = sys::ch::read_codepoint(std::span(buf1), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp1 == sys::ch::replacement<char32_t>()[0]);

        // 2-byte sequence with bad continuation.
        char8_t buf2[] { 0xC2, 0x41 };
        auto [cp2, size2] = sys::ch::read_codepoint(std::span(buf2), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp2 == sys::ch::replacement<char32_t>()[0]);
    }

    SECTION("Surrogates in UTF-8 (Illegal)")
    {
        // U+D800 encoded as UTF-8 (`0xED` `0xA0` `0x80`) should be rejected.
        char8_t buf[] { 0xED, 0xA0, 0x80 };
        auto [cp, size] = sys::ch::read_codepoint(std::span(buf), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp == sys::ch::replacement<char32_t>()[0]);
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::ch::read_codepoint` UTF-16 Exhaustive", "[sys.Text][char][utf16][decode]")
{
    SECTION("Valid Sequences")
    {
        // BMP
        char16_t buf[] { u'A' };
        auto [cp, size] = sys::ch::read_codepoint(std::span(buf), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp == U'A');
        CHECK(size == 1_uz);

        // Supplementary
        char16_t buf2[] { 0xD83D, 0xDD25 };                                     // u"🔥" (U+1F525)
        auto [cp2, size2] = sys::ch::read_codepoint(std::span(buf2), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp2 == U'🔥');
        CHECK(size2 == 2_uz);
    }

    SECTION("Unpaired Surrogates")
    {
        // Lead at end of buffer.
        char16_t buf1[] { 0xD83D };
        auto [cp1, size1] = sys::ch::read_codepoint(std::span(buf1), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp1 == sys::ch::replacement<char32_t>()[0]);

        // Lead followed by another lead.
        char16_t buf2[] { 0xD83D, 0xD83D };
        auto [cp2, size2] = sys::ch::read_codepoint(std::span(buf2), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(cp2 == sys::ch::replacement<char32_t>()[0]);

        // Trail surrogate first.
        char16_t buf3[] { 0xDD25 };
        auto [cp3, size3] = sys::ch::read_codepoint(std::span(buf3), unsafe()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(_as(uint32_t, cp3) == _as(uint32_t, sys::ch::replacement<char32_t>()[0]));
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::ch::write_codepoint` Exhaustive", "[sys.Text][char][encode]")
{
    SECTION("To UTF-8")
    {
        char8_t out[4] {};

        // 1-byte.
        CHECK(sys::ch::write_codepoint(U'A', out, unsafe()) == 1_uz);
        CHECK(out[0] == 0x41);

        // 4-byte.
        CHECK(sys::ch::write_codepoint(U'🔥', out, unsafe()) == 4_uz);
        CHECK(out[0] == 0xF0);
        CHECK(out[1] == 0x9F);
        CHECK(out[2] == 0x94);
        CHECK(out[3] == 0xA5);

        // Invalid (Surrogate) -> U+FFFD.
        CHECK(sys::ch::write_codepoint(char32_t(0xD800), out, unsafe()) == 3_uz);
        CHECK(out[0] == 0xEF);
        CHECK(out[1] == 0xBF);
        CHECK(out[2] == 0xBD);
    }

    SECTION("To UTF-16")
    {
        char16_t out[2] {};

        // BMP
        CHECK(sys::ch::write_codepoint(U'A', out, unsafe()) == 1_uz);
        CHECK(out[0] == u'A');

        // Supplementary
        CHECK(sys::ch::write_codepoint(U'🔥', out, unsafe()) == 2_uz);
        CHECK(out[0] == 0xD83D);
        CHECK(out[1] == 0xDD25);

        // Invalid (Out of Range) -> U+FFFD
        CHECK(sys::ch::write_codepoint(0x110000, out, unsafe()) == 1_uz);
        CHECK(out[0] == 0xFFFD);
    }
}

// NOLINTEND(misc-include-cleaner, readability-magic-numbers, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay)
