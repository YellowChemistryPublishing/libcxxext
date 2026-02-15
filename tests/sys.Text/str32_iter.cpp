#include <CompilerWarnings.h>
_push_nowarn_conv_comp();

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>
#include <module/sys.Text>

_pop_nowarn_conv_comp();

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("UTF-8 Iteration (Valid)", "[sys.Text][str32_iter]")
{
    sys::str s = u8"A\u00A2\u20AC\U00010348";
    sys::str32_iter<char8_t> it(s.data(), s.data() + s.size());             // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sys::str32_iter<char8_t> end(s.data() + s.size(), s.data() + s.size()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    REQUIRE(it != end);
    CHECK(*it == U'A');

    ++it;
    REQUIRE(it != end);
    CHECK(*it == U'\u00A2');

    auto prev = it++;
    CHECK(*prev == U'\u00A2');
    REQUIRE(it != end);
    CHECK(*it == U'\u20AC');

    ++it;
    REQUIRE(it != end);
    CHECK(*it == U'\U00010348');

    ++it;
    CHECK(it == end);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("UTF-16 Iteration (Valid)", "[sys.Text][str32_iter]")
{
    sys::str16 s = u"A\u00A2\u20AC\U00010348";
    sys::str32_iter<char16_t> it(s.data(), s.data() + s.size());             // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sys::str32_iter<char16_t> end(s.data() + s.size(), s.data() + s.size()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    REQUIRE(it != end);
    CHECK(*it == U'A');
    ++it;
    REQUIRE(it != end);
    CHECK(*it == U'\u00A2');
    ++it;
    REQUIRE(it != end);
    CHECK(*it == U'\u20AC');
    ++it;
    REQUIRE(it != end);
    CHECK(*it == U'\U00010348');
    ++it;
    CHECK(it == end);
}

TEST_CASE("Pathological UTF-8 Sequences", "[sys.Text][str32_iter]")
{
    // Mixed valid and invalid.
    const char8_t data[] { 0x41, 0xFF, 0x42, 0xC2 };  // NOLINT(readability-magic-numbers)
    sys::str32_iter<char8_t> it(data, data + 4);      // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay)
    sys::str32_iter<char8_t> end(data + 4, data + 4); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay)

    CHECK(*it == U'A');
    ++it;
    CHECK(*it == sys::ch::replacement<char32_t>()[0]);
    ++it;
    CHECK(*it == U'B');
    ++it;
    CHECK(*it == sys::ch::replacement<char32_t>()[0]);
    ++it;
    CHECK(it == end);
}

TEST_CASE("Iterator Comparison", "[sys.Text][str32_iter]")
{
    sys::str s = u8"test";
    sys::str32_iter<char8_t> it1(s.data(), s.data() + s.size()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sys::str32_iter<char8_t> it2(s.data(), s.data() + s.size()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    CHECK(it1 == it2);

    ++it2;
    CHECK(it1 != it2);
}

TEST_CASE("Dereference Stability", "[sys.Text][str32_iter]")
{
    sys::str s = u8"A";
    sys::str32_iter<char8_t> it(s.data(), s.data() + s.size()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    CHECK(*it == U'A');
    CHECK(*it == U'A');
}

// NOLINTEND(misc-include-cleaner)
