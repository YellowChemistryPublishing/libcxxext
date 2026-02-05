#include <iterator>
#include <ranges>
#include <string_view>

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys.Text>

// ================================================================================
// Compile-time obtainable properties. | `sys::strlit`
// ================================================================================

// Construction and basic accessors.
static_assert(sys::strlit("hello").size() == 5); // NOLINT(readability-magic-numbers)
static_assert(!sys::strlit("hello").empty());
static_assert(sys::strlit("").empty());
static_assert(sys::strlit("").size() == 0); // NOLINT(readability-container-size-empty)

// Access by index.
static_assert(sys::strlit("abc")[0] == 'a'); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
static_assert(sys::strlit("abc")[1] == 'b'); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
static_assert(sys::strlit("abc")[2] == 'c'); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

// Character types support.
static_assert(sys::strlit(L"wide").size() == 4);
static_assert(sys::strlit(u8"utf8").size() == 4);
static_assert(sys::strlit(u"utf16").size() == 5); // NOLINT(readability-magic-numbers)
static_assert(sys::strlit(U"utf32").size() == 5); // NOLINT(readability-magic-numbers)

// Buffer access.
static_assert(*sys::strlit("test").data() == 't');

// Iterator properties.
static_assert(std::forward_iterator<sys::str_liter::iter>);
static_assert(std::ranges::forward_range<sys::str_liter>);

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Runtime evaluation.", "[sys.Text][sys::strlit]")
{
    SECTION("Construct and assign.")
    {
        sys::strlit s1 = "original";
        const sys::strlit s2 = s1;
        CHECK(std::ranges::equal(s1, s2));
        CHECK(s2.data() == s1.data());

        sys::strlit s3 = std::move(s1); // NOLINT(performance-move-const-arg)
        CHECK(std::ranges::equal(s3, s2));
        CHECK(s3.data() == s2.data());

        sys::strlit s4 = "other";
        s4 = s2;
        CHECK(std::ranges::equal(s4, s2));

        sys::strlit s5 = "another";
        s5 = std::move(s3); // NOLINT(performance-move-const-arg)
        CHECK(std::ranges::equal(s5, s2));
    }

    SECTION("Basic Iteration.")
    {
        const sys::strlit s = "hello";
        CHECK(std::ranges::equal(s, std::string_view("hello")));
        CHECK(s.size() == 5); // NOLINT(readability-magic-numbers)
    }

    SECTION("Iterator operations.")
    {
        const sys::strlit s = "xyz";
        auto it = s.begin();
        CHECK(*it == 'x');
        ++it;
        CHECK(*it == 'y');
        ++it;
        CHECK(*it == 'z');
        ++it;
        CHECK(it == s.end());
    }

    SECTION("Empty string literal properties.")
    {
        const sys::strlit s = "";
        CHECK(s.empty());
        CHECK(s.size() == 0); // NOLINT(readability-container-size-empty)
        CHECK(s.begin() == s.end());
    }

    SECTION("Basic accessors.")
    {
        const sys::strlit s = "data";
        CHECK(s.data() != nullptr);
        CHECK(s[0] == 'd'); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CHECK(s[3] == 'a'); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
}

// NOLINTEND(misc-include-cleaner)
