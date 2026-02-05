// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Rounding up to a power-of-two works as expected.", "[sys][nr2ixx][i32]")
{
    SECTION("Powers-of-two return themselves.")
    {
        CHECK(sys::nr2i32(1_i32) == 1_i32);
        CHECK(sys::nr2i32(2_i32) == 2_i32);
        CHECK(sys::nr2i32(4_i32) == 4_i32);
        CHECK(sys::nr2i32(8_i32) == 8_i32);
        CHECK(sys::nr2i32(16_i32) == 16_i32);
        CHECK(sys::nr2i32(1024_i32) == 1024_i32);
        CHECK(sys::nr2i32(65536_i32) == 65536_i32);
    }

    SECTION("Non-powers-of-two round up.")
    {
        CHECK(sys::nr2i32(3_i32) == 4_i32);
        CHECK(sys::nr2i32(5_i32) == 8_i32);
        CHECK(sys::nr2i32(6_i32) == 8_i32);
        CHECK(sys::nr2i32(7_i32) == 8_i32);
        CHECK(sys::nr2i32(9_i32) == 16_i32);
        CHECK(sys::nr2i32(100_i32) == 128_i32);
        CHECK(sys::nr2i32(65535_i32) == 65536_i32);
    }

    SECTION("Zero gives zero.") { CHECK(sys::nr2i32(0_i32) == 0_i32); }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Rounding up to a power-of-two works as expected.", "[sys][nr2ixx][i64]")
{
    SECTION("Powers-of-two return themselves.")
    {
        CHECK(sys::nr2i64(1_i64) == 1_i64);
        CHECK(sys::nr2i64(2_i64) == 2_i64);
        CHECK(sys::nr2i64(4_i64) == 4_i64);
        CHECK(sys::nr2i64(8_i64) == 8_i64);
        CHECK(sys::nr2i64(16_i64) == 16_i64);
        CHECK(sys::nr2i64(1024_i64) == 1024_i64);
        CHECK(sys::nr2i64(65536_i64) == 65536_i64);
        CHECK(sys::nr2i64(4294967296_i64) == 4294967296_i64);
    }

    SECTION("Non-powers-of-two round up.")
    {
        CHECK(sys::nr2i64(3_i64) == 4_i64);
        CHECK(sys::nr2i64(5_i64) == 8_i64);
        CHECK(sys::nr2i64(6_i64) == 8_i64);
        CHECK(sys::nr2i64(7_i64) == 8_i64);
        CHECK(sys::nr2i64(9_i64) == 16_i64);
        CHECK(sys::nr2i64(100_i64) == 128_i64);
        CHECK(sys::nr2i64(65535_i64) == 65536_i64);
        CHECK(sys::nr2i64(4294967297_i64) == 8589934592_i64);
    }

    SECTION("Zero gives zero.") { CHECK(sys::nr2i64(0_i64) == 0_i64); }
}

// NOLINTEND(misc-include-cleaner)
