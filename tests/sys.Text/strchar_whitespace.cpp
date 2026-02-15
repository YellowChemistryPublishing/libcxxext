// NOLINTBEGIN(misc-include-cleaner, readability-magic-numbers, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay)

#include <catch2/catch_all.hpp>

#include <module/sys.Text>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::ch` Whitespace Verification", "[sys.Text][ch][is_whitespace]")
{
    SECTION("Standard Whitespace")
    {
        CHECK(sys::ch::is_whitespace(' '));
        CHECK(sys::ch::is_whitespace(u8'\t'));
        CHECK(sys::ch::is_whitespace(u'\n'));
        CHECK(sys::ch::is_whitespace(U'\r'));
        CHECK(sys::ch::is_whitespace(L'\f'));
        CHECK(sys::ch::is_whitespace(u8'\v'));
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
        CHECK_FALSE(sys::ch::is_whitespace(U'0'));
        CHECK_FALSE(sys::ch::is_whitespace(_as(char32_t, 0x0000))); // Null-terminator doesn't count.
        CHECK_FALSE(sys::ch::is_whitespace(_as(char32_t, 0x167F))); // Just before Ogham.
        CHECK_FALSE(sys::ch::is_whitespace(_as(char32_t, 0x1681))); // Just after Ogham.
        CHECK_FALSE(sys::ch::is_whitespace(_as(char32_t, 0x200B))); // ZERO WIDTH SPACE
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::string<...>` Trimming", "[sys.Text][string][trim]")
{
    SECTION("Basic ASCII Trimming")
    {
        CHECK(sys::str(u8"  hello  ").trim() == u8"hello");
        CHECK(sys::str(u8"  hello  ").trim_start() == u8"hello  ");
        CHECK(sys::str(u8"  hello  ").trim_end() == u8"  hello");
    }

    SECTION("Unicode Whitespace Trimming") { CHECK(sys::str(u8"\u3000\u1680Unicode\u2000\u2001").trim() == u8"Unicode"); }

    SECTION("Pathological Trimming Cases")
    {
        CHECK(sys::str(u8"   \t\n\r  ").trim().empty());
        CHECK(sys::str(u8" \u3000 \u1680 ").trim().empty());
        CHECK(sys::str(u8"word").trim() == u8"word");
        CHECK(sys::str(u8"  hello world  ").trim() == u8"hello world");
        CHECK(sys::str(u8"").trim().empty());
    }
}

// NOLINTEND(misc-include-cleaner, readability-magic-numbers, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay)
