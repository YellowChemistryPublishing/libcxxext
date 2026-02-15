#include <catch2/catch_all.hpp>

#include <module/sys.Text>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::ch::to_lower` Simple Casing", "[sys.Text][ch][to_lower]")
{
    CHECK(sys::ch::to_lower('A') == U'a');
    CHECK(sys::ch::to_lower('Z') == U'z');
    CHECK(sys::ch::to_lower('a') == U'a');
    CHECK(sys::ch::to_lower('1') == U'1');
    CHECK(sys::ch::to_lower(u8'A') == U'a');
    CHECK(sys::ch::to_lower(u8'Z') == U'z');
    CHECK(sys::ch::to_lower(u8'a') == U'a');
    CHECK(sys::ch::to_lower(u8'1') == U'1');
    CHECK(sys::ch::to_lower(U'\u03A9') == U'\u03C9'); // GREEK CAPITAL LETTER OMEGA -> SMALL
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::string::to_lower` Full Unicode Casing", "[sys.Text][string][to_lower]")
{
    SECTION("Simple and Unconditional Special")
    {
        CHECK(sys::str(u8"HELLO WORLD").to_lower() == u8"hello world");
        CHECK(sys::str16(u"HELLO WORLD").to_lower() == u"hello world");
        CHECK(sys::str32(U"HELLO WORLD").to_lower() == U"hello world");

        //             U+1E9E
        CHECK(sys::str(u8"ẞ").to_lower() == u8"ß");
        CHECK(sys::str16(u"ẞ").to_lower() == u"ß");
        CHECK(sys::str32(U"ẞ").to_lower() == U"ß");
    }

    SECTION("Final Sigma (Greek)")
    {
        // "Σ" (U+03A3) at end of word -> "ς" (U+03C2), else "σ" (U+03C3).
        CHECK(sys::str32(U"ΟΣ").to_lower() == U"ος");
        CHECK(sys::str16(u"ΟΣ").to_lower() == u"ος");
        CHECK(sys::str(u8"ΟΣ").to_lower() == u8"ος");

        CHECK(sys::str32(U"ΣΑ").to_lower() == U"σα");
        CHECK(sys::str16(u"ΣΑ").to_lower() == u"σα");
        CHECK(sys::str(u8"ΣΑ").to_lower() == u8"σα");

        CHECK(sys::str32(U"ΟΣ ΣΑ").to_lower() == U"ος σα");
        CHECK(sys::str16(u"ΟΣ ΣΑ").to_lower() == u"ος σα");
        CHECK(sys::str(u8"ΟΣ ΣΑ").to_lower() == u8"ος σα");

        CHECK(sys::str32(U"Ο'Σ").to_lower() == U"ο'ς"); // Case ignorable \' (U+0027).
        CHECK(sys::str16(u"Ο'Σ").to_lower() == u"ο'ς");
        CHECK(sys::str(u8"Ο'Σ").to_lower() == u8"ο'ς");
    }

    SECTION("Turkic (Turkish/Azeri)")
    {
        // "I" (U+0049) -> "ı" (U+0131).
        CHECK(sys::str32(U"I").to_lower(u8"tr") == U"ı");
        CHECK(sys::str16(u"I").to_lower(u8"tr") == u"ı");
        CHECK(sys::str(u8"I").to_lower(u8"tr") == u8"ı");

        // "İ" (U+0130) -> "i" (U+0069).
        CHECK(sys::str32(U"İ").to_lower(u8"tr") == U"i");
        CHECK(sys::str16(u"İ").to_lower(u8"tr") == u"i");
        CHECK(sys::str(u8"İ").to_lower(u8"tr") == u8"i");
    }

    SECTION("Lithuanian")
    {
        // "I" (U+0049) -> "i" + dot_above (U+0307) if followed by accents.
        sys::str32 s1 = U"\u00CC";
        s1.to_lower(u8"lt");
        CHECK(s1 == U"i\u0307\u0300");
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::ch::to_upper` Simple Casing", "[sys.Text][ch][to_upper]")
{
    CHECK(sys::ch::to_upper('a') == U'A');
    CHECK(sys::ch::to_upper('z') == U'Z');
    CHECK(sys::ch::to_upper('A') == U'A');
    CHECK(sys::ch::to_upper('1') == U'1');
    CHECK(sys::ch::to_upper(u8'a') == U'A');
    CHECK(sys::ch::to_upper(u8'z') == U'Z');
    CHECK(sys::ch::to_upper(u8'A') == U'A');
    CHECK(sys::ch::to_upper(u8'1') == U'1');
    CHECK(sys::ch::to_upper(U'\u03C9') == U'\u03A9'); // GREEK SMALL LETTER OMEGA -> Capitalized
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::string::to_upper` Full Unicode Casing", "[sys.Text][string][to_upper]")
{
    SECTION("Simple and Unconditional Special")
    {
        CHECK(sys::cstr("hello world").to_upper() == "HELLO WORLD");

        //             U+00DF
        CHECK(sys::str(u8"ß").to_upper() == u8"SS");
    }

    SECTION("Turkic (Turkish/Azeri)")
    {
        // "i" (U+0069) -> "İ" (U+0130).
        CHECK(sys::str32(U"i").to_upper(u8"tr") == U"İ");
        CHECK(sys::str16(u"i").to_upper(u8"tr") == u"İ");
        CHECK(sys::str(u8"i").to_upper(u8"tr") == u8"İ");

        // "ı" (U+0131) -> "I" (U+0049).
        CHECK(sys::str32(U"ı").to_upper(u8"tr") == U"I");
        CHECK(sys::str16(u"ı").to_upper(u8"tr") == u"I");
        CHECK(sys::str(u8"ı").to_upper(u8"tr") == u8"I");
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::ch::fold` Case Folding", "[sys.Text][ch][fold]")
{
    CHECK(sys::ch::fold('A') == U'a');
    CHECK(sys::ch::fold('a') == U'a');
    CHECK(sys::ch::fold(u8'A') == U'a');
    CHECK(sys::ch::fold(u8'a') == U'a');
    CHECK(sys::ch::fold(U'\u00B5') == U'\u03BC'); // MICRO SIGN -> GREEK SMALL LETTER MU
    CHECK(sys::ch::fold(U'\u0130') == U'i');      // LATIN CAPITAL LETTER I WITH DOT ABOVE -> 'i'
    CHECK(sys::ch::fold(U'\u0049') == U'\u0131'); // Turkic: "I" -> "ı"
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::string::fold` Full Case Folding", "[sys.Text][string][fold]")
{
    SECTION("Simple and Full Foldings")
    {
        CHECK(sys::str32(U"ẞ").fold() == U"ss");
        CHECK(sys::str32(U"ß").fold() == U"ss");
        CHECK(sys::str16(u"ẞ").fold() == u"ss");
        CHECK(sys::str16(u"ß").fold() == u"ss");
        CHECK(sys::str(u8"ẞ").fold() == u8"ss");
        CHECK(sys::str(u8"ß").fold() == u8"ss");

        CHECK(sys::str32(U"ﬁ").fold() == U"fi");
        CHECK(sys::str16(u"ﬁ").fold() == u"fi");
        CHECK(sys::str(u8"ﬁ").fold() == u8"fi");
    }

    SECTION("Turkic Case Folding")
    {
        CHECK(sys::str32(U"I").fold(u8"tr") == U"ı");
        CHECK(sys::str16(u"I").fold(u8"tr") == u"ı");
        CHECK(sys::str(u8"I").fold(u8"tr") == u8"ı");

        CHECK(sys::str32(U"İ").fold(u8"tr") == U"i");
        CHECK(sys::str16(u"İ").fold(u8"tr") == u"i");
        CHECK(sys::str(u8"İ").fold(u8"tr") == u8"i");
    }

    SECTION("Context Independence")
    {
        // Sigma should always fold to small sigma (U+03C3), unlike `.to_lower()` which uses final sigma (U+03C2).
        CHECK(sys::str32(U"Σ").fold() == U"σ");
        CHECK(sys::str16(u"Σ").fold() == u"σ");
        CHECK(sys::str(u8"Σ").fold() == u8"σ");

        CHECK(sys::str32(U"ΑΣ").fold() == U"ασ"); // Not "ας".
        CHECK(sys::str16(u"ΑΣ").fold() == u"ασ");
        CHECK(sys::str(u8"ΑΣ").fold() == u8"ασ");
    }
}
