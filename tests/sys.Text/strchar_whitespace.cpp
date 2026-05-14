// NOLINTBEGIN(bugprone-throwing-static-initialization, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay, misc-include-cleaner)
// NOLINTBEGIN(readability-function-cognitive-complexity, readability-magic-numbers)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys.Text>

TEST_CASE("Standard Whitespace", "[sys.Text][ch][is_whitespace]")
{
    CHECK(sys::ch::is_whitespace(' '));
    CHECK(sys::ch::is_whitespace(u8'\t'));
    CHECK(sys::ch::is_whitespace(u'\n'));
    CHECK(sys::ch::is_whitespace(U'\r'));
    CHECK(sys::ch::is_whitespace(L'\f'));
    CHECK(sys::ch::is_whitespace(u8'\v'));
}

TEST_CASE("Unicode Whitespace (Exhaustive)", "[sys.Text][ch][is_whitespace]")
{
    // Space Separators
    CHECK(sys::ch::is_whitespace(_as(0x00A0, char32_t))); // NBSP
    CHECK(sys::ch::is_whitespace(_as(0x1680, char32_t))); // OGHAM SPACE MARK
    CHECK(sys::ch::is_whitespace(_as(0x2000, char32_t))); // EN QUAD
    CHECK(sys::ch::is_whitespace(_as(0x2001, char32_t))); // EM QUAD
    CHECK(sys::ch::is_whitespace(_as(0x2002, char32_t))); // EN SPACE
    CHECK(sys::ch::is_whitespace(_as(0x2003, char32_t))); // EM SPACE
    CHECK(sys::ch::is_whitespace(_as(0x2004, char32_t))); // THREE-PER-EM SPACE
    CHECK(sys::ch::is_whitespace(_as(0x2005, char32_t))); // FOUR-PER-EM SPACE
    CHECK(sys::ch::is_whitespace(_as(0x2006, char32_t))); // SIX-PER-EM SPACE
    CHECK(sys::ch::is_whitespace(_as(0x2007, char32_t))); // FIGURE SPACE
    CHECK(sys::ch::is_whitespace(_as(0x2008, char32_t))); // PUNCTUATION SPACE
    CHECK(sys::ch::is_whitespace(_as(0x2009, char32_t))); // THIN SPACE
    CHECK(sys::ch::is_whitespace(_as(0x200A, char32_t))); // HAIR SPACE
    CHECK(sys::ch::is_whitespace(_as(0x202F, char32_t))); // NARROW NO-BREAK SPACE
    CHECK(sys::ch::is_whitespace(_as(0x205F, char32_t))); // MEDIUM MATHEMATICAL SPACE
    CHECK(sys::ch::is_whitespace(_as(0x3000, char32_t))); // IDEOGRAPHIC SPACE

    // Line / Paragraph Separators
    CHECK(sys::ch::is_whitespace(_as(0x2028, char32_t))); // LINE SEPARATOR
    CHECK(sys::ch::is_whitespace(_as(0x2029, char32_t))); // PARAGRAPH SEPARATOR

    // Other Control Characters
    CHECK(sys::ch::is_whitespace(_as(0x0085, char32_t))); // NEXT LINE (NEL)
}

TEST_CASE("Non-Whitespace Boundaries", "[sys.Text][ch][is_whitespace]")
{
    CHECK_FALSE(sys::ch::is_whitespace('A'));
    CHECK_FALSE(sys::ch::is_whitespace(U'0'));
    CHECK_FALSE(sys::ch::is_whitespace(_as(0x0000, char32_t))); // Null-terminator doesn't count.
    CHECK_FALSE(sys::ch::is_whitespace(_as(0x167F, char32_t))); // Just before Ogham.
    CHECK_FALSE(sys::ch::is_whitespace(_as(0x1681, char32_t))); // Just after Ogham.
    CHECK_FALSE(sys::ch::is_whitespace(_as(0x200B, char32_t))); // ZERO WIDTH SPACE
}

TEST_CASE("Basic ASCII String Trimming", "[sys.Text][string][trim]")
{
    CHECK(sys::str(u8"  hello  ").trim() == u8"hello");
    CHECK(sys::str(u8"  hello  ").trim_start() == u8"hello  ");
    CHECK(sys::str(u8"  hello  ").trim_end() == u8"  hello");
}

TEST_CASE("Unicode Whitespace Trimming", "[sys.Text][string][trim]") { CHECK(sys::str(u8"\u3000\u1680Unicode\u2000\u2001").trim() == u8"Unicode"); }

TEST_CASE("Pathological Trimming Cases", "[sys.Text][string][trim]")
{
    CHECK(sys::str(u8"   \t\n\r  ").trim().empty());
    CHECK(sys::str(u8" \u3000 \u1680 ").trim().empty());
    CHECK(sys::str(u8"word").trim() == u8"word");
    CHECK(sys::str(u8"  hello world  ").trim() == u8"hello world");
    CHECK(sys::str(u8"").trim().empty());
}

// NOLINTEND(readability-function-cognitive-complexity, readability-magic-numbers)
// NOLINTEND(bugprone-throwing-static-initialization, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay, misc-include-cleaner)
