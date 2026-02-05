// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Byte manipulation works as expected.", "[sys][xfxx]")
{
    SECTION("Correctly construct `i16` from two bytes.")
    {
        CHECK(sys::s16fb2(0_u8, 0_u8) == 0_i16);
        CHECK(sys::s16fb2(0_u8, 1_u8) == 1_i16);
        CHECK(sys::s16fb2(1_u8, 0_u8) == 256_i16);
        CHECK(sys::s16fb2(0xFF_u8, 0xFF_u8) == -1_i16);
        CHECK(sys::s16fb2(0x7F_u8, 0xFF_u8) == 32767_i16);
        CHECK(sys::s16fb2(0x80_u8, 0x00_u8) == i16(-32768));
    }

    SECTION("Correctly extract high byte.")
    {
        CHECK(sys::hbfs16(0_i16) == 0_u8);
        CHECK(sys::hbfs16(256_i16) == 1_u8);
        CHECK(sys::hbfs16(-1_i16) == 0xFF_u8);
        CHECK(sys::hbfs16(32767_i16) == 0x7F_u8);
        CHECK(sys::hbfs16(i16(-32768)) == 0x80_u8);
    }

    SECTION("Correctly extract low byte.")
    {
        CHECK(sys::lbfs16(0_i16) == 0_u8);
        CHECK(sys::lbfs16(1_i16) == 1_u8);
        CHECK(sys::lbfs16(256_i16) == 0_u8);
        CHECK(sys::lbfs16(-1_i16) == 0xFF_u8);
        CHECK(sys::lbfs16(32767_i16) == 0xFF_u8);
        CHECK(sys::lbfs16(i16(-32768)) == 0x00_u8);
    }
}

// NOLINTEND(misc-include-cleaner)
