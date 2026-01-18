// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <cxxsup.h>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Rounding up to a power-of-two works as expected. | `sys::nr2i32(...)`")
{
    SECTION("Powers-of-two return themselves.")
    {
        CHECK(sys::nr2i32(1) == 1);
        CHECK(sys::nr2i32(2) == 2);
        CHECK(sys::nr2i32(4) == 4);
        CHECK(sys::nr2i32(8) == 8);
        CHECK(sys::nr2i32(16) == 16);
        CHECK(sys::nr2i32(1024) == 1024);
        CHECK(sys::nr2i32(65536) == 65536);
    }

    SECTION("Non-powers-of-two round up.")
    {
        CHECK(sys::nr2i32(3) == 4);
        CHECK(sys::nr2i32(5) == 8);
        CHECK(sys::nr2i32(6) == 8);
        CHECK(sys::nr2i32(7) == 8);
        CHECK(sys::nr2i32(9) == 16);
        CHECK(sys::nr2i32(100) == 128);
        CHECK(sys::nr2i32(65535) == 65536);
    }

    SECTION("Zero gives zero.")
    {
        CHECK(sys::nr2i32(0) == 0);
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Rounding up to a power-of-two works as expected. | `sys::nr2i64(...)`")
{
    SECTION("Powers-of-two return themselves.")
    {
        CHECK(sys::nr2i64(1ll) == 1ll);
        CHECK(sys::nr2i64(2ll) == 2ll);
        CHECK(sys::nr2i64(4ll) == 4ll);
        CHECK(sys::nr2i64(8ll) == 8ll);
        CHECK(sys::nr2i64(16ll) == 16ll);
        CHECK(sys::nr2i64(1024ll) == 1024ll);
        CHECK(sys::nr2i64(65536ll) == 65536ll);
        CHECK(sys::nr2i64(4294967296ll) == 4294967296ll);
    }

    SECTION("Non-powers-of-two round up.")
    {
        CHECK(sys::nr2i64(3ll) == 4ll);
        CHECK(sys::nr2i64(5ll) == 8ll);
        CHECK(sys::nr2i64(6ll) == 8ll);
        CHECK(sys::nr2i64(7ll) == 8ll);
        CHECK(sys::nr2i64(9ll) == 16ll);
        CHECK(sys::nr2i64(100ll) == 128ll);
        CHECK(sys::nr2i64(65535ll) == 65536ll);
        CHECK(sys::nr2i64(4294967297ll) == 8589934592ll);
    }

    SECTION("Zero gives zero.")
    {
        CHECK(sys::nr2i64(0) == 0);
    }
}

// NOLINTEND(misc-include-cleaner)
