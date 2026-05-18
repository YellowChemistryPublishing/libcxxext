// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("s16fb2(...)", "[sys][s16fb2]")
{
    CHECK(sys::s16fb2(0_u8, 0_u8) == 0_i16);
    CHECK(sys::s16fb2(0_u8, 1_u8) == 1_i16);
    CHECK(sys::s16fb2(1_u8, 0_u8) == 256_i16);
    CHECK(sys::s16fb2(0xFF_u8, 0xFF_u8) == -1_i16);
    CHECK(sys::s16fb2(0x7F_u8, 0xFF_u8) == 32767_i16);
    CHECK(sys::s16fb2(0x80_u8, 0x00_u8) == i16(-32768));
}
TEST_CASE("hbfs16(...)", "[sys][hbfs16]")
{
    CHECK(sys::hbfs16(0_i16) == 0_u8);
    CHECK(sys::hbfs16(256_i16) == 1_u8);
    CHECK(sys::hbfs16(-1_i16) == 0xFF_u8);
    CHECK(sys::hbfs16(32767_i16) == 0x7F_u8);
    CHECK(sys::hbfs16(i16(-32768)) == 0x80_u8);
}
TEST_CASE("lbfs16(...)", "[sys][lbfs16]")
{
    CHECK(sys::lbfs16(0_i16) == 0_u8);
    CHECK(sys::lbfs16(1_i16) == 1_u8);
    CHECK(sys::lbfs16(256_i16) == 0_u8);
    CHECK(sys::lbfs16(-1_i16) == 0xFF_u8);
    CHECK(sys::lbfs16(32767_i16) == 0xFF_u8);
    CHECK(sys::lbfs16(i16(-32768)) == 0x00_u8);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
