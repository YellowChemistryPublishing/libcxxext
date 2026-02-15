#include <CompilerWarnings.h>

_push_nowarn_conv_comp();

#include <utility>

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

#include <module/sys>
#include <module/sys.Text>

_pop_nowarn_conv_comp();

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::string<...>` Constructors", "[sys.Text][string][ctor]")
{
    SECTION("Empty Constructors")
    {
        const sys::str s;
        CHECK(s.empty());
        CHECK(s.size() == _as(sz, 0_uz));
    }

    SECTION("Literal Constructors")
    {
        sys::cstr s = "hello";
        CHECK(s == "hello");
        CHECK(s.size() == _as(sz, 5_uz));

        sys::str16 s16 = u"world";
        CHECK(s16 == u"world");
        CHECK(s16.size() == _as(sz, 5_uz));
    }

    SECTION("Initializer List")
    {
        sys::str s { u8'a', u8'b', u8'c' };
        CHECK(s == u8"abc");
    }

    SECTION("Unsafe C-String Constructor")
    {
        sys::str s(u8"unsafe", unsafe());
        CHECK(s == u8"unsafe");

        const sys::str empty(_as(const char8_t*, nullptr), unsafe());
        CHECK(empty.empty());
    }

    SECTION("Span Constructor")
    {
        char8_t data[] = { u8'h', u8'i' };
        CHECK(sys::str(std::span<char8_t>(data)) == u8"hi");
    }

    SECTION("Copy and Move")
    {
        sys::str s1 = u8"test";
        sys::str s2 = s1;
        CHECK(s2 == u8"test");

        sys::str s3 = std::move(s1);
        CHECK(s3 == u8"test");
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::string<...>` Splitting and Joining", "[sys.Text][string][split][join]")
{
    SECTION("Split by Character")
    {
        auto parts = sys::str(u8"a,b,c").split(u8',');
        REQUIRE(parts.size() == 3_uz);
        CHECK(parts[0] == u8"a");
    }

    SECTION("Split with Adjacent Delimiters")
    {
        auto parts = sys::str(u8"a,,b").split(u8',');
        REQUIRE(parts.size() == 3_uz);
        CHECK(parts[1] == u8"");
    }

    SECTION("Split by String Delimiter")
    {
        auto parts = sys::str(u8"one--two--three").split(std::basic_string_view<char8_t>(u8"--"));
        REQUIRE(parts.size() == 3_uz);
        CHECK(parts[1] == u8"two");
    }

    SECTION("Join Strings")
    {
        CHECK(sys::str::join(std::vector<sys::str> { u8"a", u8"b", u8"c" }, u8", ") == u8"a, b, c");
        CHECK(sys::str::join({ u8"", u8"" }, u8",") == u8",");
    }
}

TEST_CASE("`sys::string<...>` Transcoding", "[sys.Text][string][conv]")
{
    SECTION("UTF-8 -> UTF-(16|32)")
    {
        const sys::str s = u8"A\u00A2\u20AC\U00010348";
        CHECK(sys::str16(s) == u"A\u00A2\u20AC\U00010348");
        CHECK(sys::str32(s) == U"A\u00A2\u20AC\U00010348");
        CHECK(sys::cstr(s) == "A\u00A2\u20AC\U00010348");
        CHECK(sys::wstr(s) == L"A\u00A2\u20AC\U00010348");
    }
}

TEST_CASE("`sys::string<...>` Predicates", "[sys.Text][string][predicate]")
{
    const sys::str s = u8"Hello World";
    CHECK(s.starts_with(u8"Hello"));
    CHECK(s.ends_with(u8"World"));
    CHECK(s.contains(u8"lo Wo"));
    CHECK_FALSE(s.contains(u8"uh oh"));
}

TEST_CASE("`sys::string<...>` Iterator Boundaries", "[sys.Text][string][iter]")
{
    const sys::str s = u8"abc";
    CHECK(s.begin() != s.end());
    CHECK(*s.begin() == u8'a');
    CHECK(*(s.end() - 1z) == u8'c');
}

// NOLINTEND(misc-include-cleaner)
