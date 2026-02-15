#include <CompilerWarnings.h>

_push_nowarn_conv_comp();

#include <utility>

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

#include <module/sys>
#include <module/sys.Text>

_pop_nowarn_conv_comp();

TEST_CASE("Empty Constructors", "[sys.Text][string][ctor]")
{
    const sys::str s;
    CHECK(s.empty());
    CHECK(s.size() == _as(sz, 0_uz));
}

TEST_CASE("Literal Constructors", "[sys.Text][string][ctor]")
{
    sys::cstr s = "hello";
    CHECK(s == "hello");
    CHECK(s.size() == _as(sz, 5_uz));

    sys::str16 s16 = u"world";
    CHECK(s16 == u"world");
    CHECK(s16.size() == _as(sz, 5_uz));
}

TEST_CASE("Initializer List", "[sys.Text][string][ctor]")
{
    sys::str s { u8'a', u8'b', u8'c' };
    CHECK(s == u8"abc");
}

TEST_CASE("Unsafe C-String Constructor", "[sys.Text][string][ctor]")
{
    sys::str s(u8"unsafe", unsafe());
    CHECK(s == u8"unsafe");

    const sys::str empty(_as(const char8_t*, nullptr), unsafe());
    CHECK(empty.empty());
}

TEST_CASE("Span Constructor", "[sys.Text][string][ctor]")
{
    char8_t data[] = { u8'h', u8'i' };
    CHECK(sys::str(std::span<char8_t>(data)) == u8"hi");
}

TEST_CASE("Copy and Move", "[sys.Text][string][ctor]")
{
    sys::str s1 = u8"test";
    sys::str s2 = s1;
    CHECK(s2 == u8"test");

    sys::str s3 = std::move(s1);
    CHECK(s3 == u8"test");
}

TEST_CASE("Split by Character", "[sys.Text][string][split]")
{
    auto parts = sys::str(u8"a,b,c").split(u8',');
    REQUIRE(parts.size() == 3_uz);
    CHECK(parts[0] == u8"a");
}

TEST_CASE("Split with Adjacent Delimiters", "[sys.Text][string][split]")
{
    auto parts = sys::str(u8"a,,b").split(u8',');
    REQUIRE(parts.size() == 3_uz);
    CHECK(parts[1] == u8"");
}

TEST_CASE("Split by String Delimiter", "[sys.Text][string][split]")
{
    auto parts = sys::str(u8"one--two--three").split(std::basic_string_view<char8_t>(u8"--"));
    REQUIRE(parts.size() == 3_uz);
    CHECK(parts[1] == u8"two");
}

TEST_CASE("Split by Empty String Delimiter", "[sys.Text][string][split]")
{
    auto parts = sys::str(u8"abcde").split(std::basic_string_view<char8_t>(u8""));
    REQUIRE(parts.size() == 5_uz);
    CHECK(parts == std::vector<sys::str> { u8"a", u8"b", u8"c", u8"d", u8"e" });
}

TEST_CASE("Join Strings", "[sys.Text][string][join]")
{
    CHECK(sys::str::join(std::vector<sys::str> { u8"a", u8"b", u8"c" }, u8", ") == u8"a, b, c");
    CHECK(sys::str::join({ u8"", u8"" }, u8",") == u8",");
}

TEST_CASE("Transcoding UTF-8 -> UTF-(16|32)", "[sys.Text][string][conv]")
{
    const sys::str s = u8"A\u00A2\u20AC\U00010348";
    CHECK(sys::str16(s) == u"A\u00A2\u20AC\U00010348");
    CHECK(sys::str32(s) == U"A\u00A2\u20AC\U00010348");
    CHECK(sys::cstr(s) == "A\u00A2\u20AC\U00010348");
    CHECK(sys::wstr(s) == L"A\u00A2\u20AC\U00010348");
}

TEST_CASE("Predicates", "[sys.Text][string][predicate]")
{
    const sys::str s = u8"Hello World";
    CHECK(s.starts_with(u8"Hello"));
    CHECK(s.ends_with(u8"World"));
    CHECK(s.contains(u8"lo Wo"));
    CHECK_FALSE(s.contains(u8"uh oh"));
}

TEST_CASE("Iterator Boundaries", "[sys.Text][string][iter]")
{
    const sys::str s = u8"abc";
    CHECK(s.begin() != s.end());
    CHECK(*s.begin() == u8'a');
    CHECK(*(s.end() - 1z) == u8'c');
}

TEST_CASE("Format", "[sys.Text][string][format]")
{
    CHECK(std::format("{}", sys::str(u8"beans")) == "beans");
    CHECK(std::format("{}", sys::str16(u"beans")) == "beans");
    CHECK(std::format("{}", sys::str32(U"beans")) == "beans");
    CHECK(std::format("{}", sys::cstr("beans")) == "beans");
    CHECK(std::format("{}", sys::wstr(L"beans")) == "beans");

    CHECK(std::format(L"{}", sys::str(u8"beans")) == L"beans");
    CHECK(std::format(L"{}", sys::str16(u"beans")) == L"beans");
    CHECK(std::format(L"{}", sys::str32(U"beans")) == L"beans");
    CHECK(std::format(L"{}", sys::cstr("beans")) == L"beans");
    CHECK(std::format(L"{}", sys::wstr(L"beans")) == L"beans");
}

// NOLINTEND(misc-include-cleaner)
