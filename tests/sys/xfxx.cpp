// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <cxxsup.h>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Byte manipulation works as expected. | `sys::s16fb2(...)`, `sys::hbfs16(...)`, `sys::lbfs16(...)`")
{
    SECTION("Correctly construct `i16` from two bytes.")
    {
        CHECK(sys::s16fb2(0, 0) == 0);
        CHECK(sys::s16fb2(0, 1) == 1);
        CHECK(sys::s16fb2(1, 0) == 256);
        CHECK(sys::s16fb2(0xFF, 0xFF) == -1);
        CHECK(sys::s16fb2(0x7F, 0xFF) == 32767);
        CHECK(sys::s16fb2(0x80, 0x00) == -32768);
    }

    SECTION("Correctly extract high byte.")
    {
        CHECK(sys::hbfs16(0) == 0);
        CHECK(sys::hbfs16(256) == 1);
        CHECK(sys::hbfs16(-1) == 0xFF);
        CHECK(sys::hbfs16(32767) == 0x7F);
        CHECK(sys::hbfs16(-32768) == 0x80);
    }

    SECTION("Correctly extract low byte.")
    {
        CHECK(sys::lbfs16(0) == 0);
        CHECK(sys::lbfs16(1) == 1);
        CHECK(sys::lbfs16(256) == 0);
        CHECK(sys::lbfs16(-1) == 0xFF);
        CHECK(sys::lbfs16(32767) == 0xFF);
        CHECK(sys::lbfs16(-32768) == 0x00);
    }
}

// NOLINTEND(misc-include-cleaner)
