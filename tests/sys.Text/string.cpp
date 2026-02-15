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
        sys::str s = u8"hello";
        CHECK(s == u8"hello");
        CHECK(s.size() == _as(sz, 5_uz));

        sys::str16 s16 = u"world";
        CHECK(s16 == u"world");
        CHECK(s16.size() == _as(sz, 5_uz));
    }

    SECTION("Initializer List")
    {
        sys::str s = { u8'a', u8'b', u8'c' };
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
        sys::str s { std::span<char8_t> { data } };
        CHECK(s == u8"hi");
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
        const sys::str s = u8"a,b,c";
        auto parts = s.split(u8',');

        REQUIRE(parts.size() == 3_uz);
        CHECK(parts[0] == u8"a");
    }

    SECTION("Split with Adjacent Delimiters")
    {
        const sys::str s = u8"a,,b";
        auto parts = s.split(u8',');

        REQUIRE(parts.size() == 3_uz);
        CHECK(parts[1] == u8"");
    }

    SECTION("Split by String Delimiter")
    {
        const sys::str s = u8"one--two--three";
        auto parts = s.split(std::basic_string_view<char8_t>(u8"--"));

        REQUIRE(parts.size() == 3_uz);
        CHECK(parts[1] == u8"two");
    }

    SECTION("Join Strings")
    {
        const std::vector<sys::str> parts = { u8"a", u8"b", u8"c" };
        CHECK(sys::str::join(parts, u8", ") == u8"a, b, c");

        const std::vector<sys::str> empties = { u8"", u8"" };
        CHECK(sys::str::join(empties, u8",") == u8",");
    }
}

TEST_CASE("`sys::string<...>` Transcoding", "[sys.Text][string][conv]")
{
    SECTION("UTF-8 -> UTF-(16|32)")
    {
        const sys::str s8 = u8"A\u00A2\u20AC\U00010348";

        sys::str16 s16(s8);
        CHECK(s16 == u"A\u00A2\u20AC\U00010348");

        sys::str32 s32(s8);
        CHECK(s32 == U"A\u00A2\u20AC\U00010348");
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
