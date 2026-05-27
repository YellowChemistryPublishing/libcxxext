// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("bit_ceil<...>(...), x = 0 => bit_ceil(x) == 0, x = 2^n => bit_ceil(x) == 2^n", "[sys][bit_ceil]")
{
    CHECK(sys::bit_ceil(1_i64) == 1_i64);
    CHECK(sys::bit_ceil(2_i64) == 2_i64);
    CHECK(sys::bit_ceil(4_i64) == 4_i64);
    CHECK(sys::bit_ceil(8_i64) == 8_i64);
    CHECK(sys::bit_ceil(16_i64) == 16_i64);
    CHECK(sys::bit_ceil(1024_i64) == 1024_i64);
    CHECK(sys::bit_ceil(65536_i64) == 65536_i64);
    CHECK(sys::bit_ceil(4294967296_i64) == 4294967296_i64);
}
TEST_CASE("bit_ceil<...>(...), 2^n < x < 2^(n + 1) => bit_ceil(x) == 2^(n + 1)", "[sys][bit_ceil]")
{
    CHECK(sys::bit_ceil(3_i64) == 4_i64);
    CHECK(sys::bit_ceil(5_i64) == 8_i64);
    CHECK(sys::bit_ceil(6_i64) == 8_i64);
    CHECK(sys::bit_ceil(7_i64) == 8_i64);
    CHECK(sys::bit_ceil(9_i64) == 16_i64);
    CHECK(sys::bit_ceil(100_i64) == 128_i64);
    CHECK(sys::bit_ceil(65535_i64) == 65536_i64);
    CHECK(sys::bit_ceil(4294967297_i64) == 8589934592_i64);
}
TEST_CASE("bit_ceil<...>(...), x <= 0, x > max(2^n) => bit_ceil(x) == 0", "[sys][bit_ceil]")
{
    CHECK(sys::bit_ceil(i64::lowest()) == 0_i64);
    CHECK(sys::bit_ceil(-4294967297_i64) == 0_i64);
    CHECK(sys::bit_ceil(-4294967296_i64) == 0_i64);
    CHECK(sys::bit_ceil(-1_i64) == 0_i64);
    CHECK(sys::bit_ceil(-3_i64) == 0_i64);
    CHECK(sys::bit_ceil(0_i64) == 0_i64);
    CHECK(sys::bit_ceil(i64::highest()) == 0_i64);
    CHECK(sys::bit_ceil(i64::highest() - 2048_i64) == 0_i64);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
