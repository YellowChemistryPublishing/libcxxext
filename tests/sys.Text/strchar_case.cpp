#include <catch2/catch_all.hpp>

#include <module/sys.Text>

TEST_CASE("`sys::ch::to_lower` Simple Casing", "[sys.Text][ch][to_lower]")
{
    CHECK(sys::ch::to_lower('A') == 'a');
    CHECK(sys::ch::to_lower('Z') == 'z');
    CHECK(sys::ch::to_lower('a') == 'a');
    CHECK(sys::ch::to_lower('1') == '1');
    CHECK(sys::ch::to_lower(U'\u03A9') == U'\u03C9'); // GREEK CAPITAL LETTER OMEGA -> SMALL
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::string::to_lower` Full Unicode Casing", "[sys.Text][string][to_lower]")
{
    SECTION("Simple and Unconditional Special")
    {
        sys::str s = u8"HELLO WORLD";
        s.to_lower();
        CHECK(s == u8"hello world");

        sys::str32 s2 = U"ẞ"; // U+1E9E
        s2.to_lower();
        CHECK(s2 == U"ß");
    }

    SECTION("Final Sigma (Greek)")
    {
        // "Σ" (U+03A3) at end of word -> "ς" (U+03C2), else "σ" (U+03C3).
        sys::str32 s1 = U"ΟΣ";
        s1.to_lower();
        CHECK(s1 == U"ος");

        sys::str32 s2 = U"ΣΑ";
        s2.to_lower();
        CHECK(s2 == U"σα");

        sys::str32 s3 = U"ΟΣ ΣΑ";
        s3.to_lower();
        CHECK(s3 == U"ος σα");

        sys::str32 s4 = U"Ο'Σ"; // Case ignorable \' (U+0027).
        s4.to_lower();
        CHECK(s4 == U"ο'ς");
    }

    SECTION("Turkic (Turkish/Azeri)")
    {
        // "I" (U+0049) -> "ı" (U+0131).
        sys::str32 s1 = U"I";
        s1.to_lower(u8"tr");
        CHECK(s1 == U"ı");

        // "İ" (U+0130) -> "i" (U+0069).
        sys::str32 s2 = U"İ";
        s2.to_lower(u8"tr");
        CHECK(s2 == U"i");
    }

    SECTION("Lithuanian")
    {
        // "I" (U+0049) -> "i" + dot_above (U+0307) if followed by accents.
        sys::str32 s1 = U"\u00CC";
        s1.to_lower(u8"lt");
        CHECK(s1 == U"i\u0307\u0300");
    }
}

TEST_CASE("`sys::ch::to_upper` Simple Casing", "[sys.Text][ch][to_upper]")
{
    CHECK(sys::ch::to_upper('a') == 'A');
    CHECK(sys::ch::to_upper('z') == 'Z');
    CHECK(sys::ch::to_upper('A') == 'A');
    CHECK(sys::ch::to_upper('1') == '1');
    CHECK(sys::ch::to_upper(U'\u03C9') == U'\u03A9'); // GREEK SMALL LETTER OMEGA -> Capitalized
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::string::to_upper` Full Unicode Casing", "[sys.Text][string][to_upper]")
{
    SECTION("Simple and Unconditional Special")
    {
        sys::str s = u8"hello world";
        s.to_upper();
        CHECK(s == u8"HELLO WORLD");

        sys::str32 s2 = U"ß"; // U+00DF
        s2.to_upper();
        CHECK(s2 == U"SS");
    }

    SECTION("Turkic (Turkish/Azeri)")
    {
        // "i" (U+0069) -> "İ" (U+0130).
        sys::str32 s1 = U"i";
        s1.to_upper(u8"tr");
        CHECK(s1 == U"İ");

        // "ı" (U+0131) -> "I" (U+0049).
        sys::str32 s2 = U"ı";
        s2.to_upper(u8"tr");
        CHECK(s2 == U"I");
    }
}
