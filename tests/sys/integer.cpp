#include <cstdint>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Properties.", "[sys][integer]")
{
    static_assert(sizeof(sys::integer<uint_least8_t>) == sizeof(uint_least8_t));
    static_assert(alignof(sys::integer<uint_least8_t>) == alignof(uint_least8_t));

    CHECK_FALSE(u32::is_signed());
    CHECK(i8::is_signed());
    CHECK(i8::highest() == 127_i8);      // NOLINT(readability-magic-numbers)
    CHECK(i16::lowest() == i16(-32768)); // NOLINT(readability-magic-numbers)
    CHECK(u8::ones() == 0b11111111_u8);  // NOLINT(readability-magic-numbers)
    CHECK(i8::sentinel() == sys::bsentinel<i8::underlying_type>());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Construction and assignment.", "[sys][integer]")
{
    CHECK(i16() == 0);
    CHECK(i64(1) == 1);
    CHECK(i8(127) == 127);
    CHECK(u8(128.0f /* NOLINT(readability-magic-numbers) */) == 128);
    CHECK(u8(0b111111111, unsafe) == 0xFF);

    const i8 a(-128 /* NOLINT(readability-magic-numbers) */);
    const u64 b = 0xFFFFFFFFFFFFFFFF_u64 /* NOLINT(readability-magic-numbers) */;
    u32 c(a);
    CHECK(c == 0);
    u32 d(b, unsafe);
    CHECK(d == 0xFFFFFFFF_u32);

    i16 x = 3_i16;
    i16 y(x);
    CHECK(x == y);
    x = 4_i16;
    y = x;
    CHECK(x == y);
    y = _as(2, byte);
    CHECK(y == 2);

    CHECK(*x == _as(x, i16::underlying_type));
    CHECK(!!(y == *y));
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Testability and comparability.", "[sys][integer]")
{
    const i32 x = -1_i32, y = 1_i32;

    CHECK(x);
    CHECK(y);
    CHECK(!0_i32);
    CHECK(!i32());

    CHECK(x == x);
    CHECK(x != y);
    CHECK(x < y);
    CHECK(x <= y);
    CHECK(x <= -1_i32);
    CHECK_FALSE(x > y);
    CHECK_FALSE(x >= y);
    CHECK(x >= -1_i32);

    CHECK(x == *x);
    CHECK(x != *y);
    CHECK(x < *y);
    CHECK(x <= *y);
    CHECK(x <= *-1_i32);
    CHECK_FALSE(x > *y);
    CHECK_FALSE(x >= *y);
    CHECK(x >= *-1_i32);

    CHECK(*x == x);
    CHECK(*x != y);
    CHECK(*x < y);
    CHECK(*x <= y);
    CHECK(*x <= -1_i32);
    CHECK_FALSE(*x > y);
    CHECK_FALSE(*x >= y);
    CHECK(*x >= -1_i32);
}

TEST_CASE("Unary arithmetic operators.", "[sys][integer]")
{
    CHECK(-i32(-32) == +32_i32);
    CHECK(-i64::lowest() == +i64::highest());
    CHECK(~u8::ones() == 0_u8);
    CHECK(_as(-1_i8, int) == -1);
    CHECK(_as(-1_i8, float) == -1.0f);

    i32 x = 0_i32;
    CHECK(++--x == 0_i32);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Binary arithmetic operators.", "[sys][integer]")
{
    CHECK(2_i32 + 3_i32 == 5_i32);
    CHECK(1_i32 - 5_i32 == -4_i32);
    CHECK(6_i32 * 9_i32 == 54_i32);
    CHECK(5_i32 / 2_i32 == 2_i32);
    CHECK(1_i32 / 0_i32 == i32::highest());
    CHECK(-1_i32 / 0_i32 == i32::lowest());
    CHECK(5_i32 % 2_i32 == 1_i32);
    CHECK(12_i32 % 0_i32 == 12_i32);

    i32 a = 2_i32;
    CHECK((a += 3_i32) == 5_i32);
    a = 1_i32;
    CHECK((a -= 5_i32) == -4_i32);
    a = 6_i32 /* NOLINT(readability-magic-numbers) */;
    CHECK((a *= 9_i32) == 54_i32);
    a = 5_i32 /* NOLINT(readability-magic-numbers) */;
    CHECK((a /= 2_i32) == 2_i32);
    a = 1_i32;
    CHECK((a /= 0_i32) == i32::highest());
    a = -1_i32;
    CHECK((a /= 0_i32) == i32::lowest());
    a = 5_i32 /* NOLINT(readability-magic-numbers) */;
    CHECK((a %= 2_i32) == 1_i32);
    a = 12_i32 /* NOLINT(readability-magic-numbers) */;
    CHECK((a %= 0_i32) == 12_i32);

    CHECK((0b1111_u8 & 0b1010_u8) == 0b1010_u8);
    CHECK((0b0101_u8 | 0b1010_u8) == 0b1111_u8);
    CHECK((0b0100_u8 ^ 0b1010_u8) == 0b1110_u8);
    CHECK((0b11110000_u8 >> 4_u8) == 0b1111_u8);
    CHECK((0b11110000_u8 << 2_u8) == 0b11000000_u8);
    CHECK((0b1111_i8 >> 3_i8) == 0b1_i8);
    CHECK((0b1111_i8 << 2_i8) == 0b111100_i8);
    CHECK((0b1111_i8 >> -1_i8) == 0b11110_i8);
    CHECK((0b1111_i8 << -2_i8) == 0b11_i8);

    u8 x = 0b1111_u8 /* NOLINT(readability-magic-numbers) */;
    CHECK((x &= 0b1010_u8) == 0b1010_u8);
    x = 0b0101_u8 /* NOLINT(readability-magic-numbers) */;
    CHECK((x |= 0b1010_u8) == 0b1111_u8);
    x = 0b0100_u8 /* NOLINT(readability-magic-numbers) */;
    CHECK((x ^= 0b1010_u8) == 0b1110_u8);
    x = 0b11110000_u8 /* NOLINT(readability-magic-numbers) */;
    CHECK((x >>= 4_u8) == 0b1111_u8);
    x = 0b11110000_u8 /* NOLINT(readability-magic-numbers) */;
    CHECK((x <<= 2_u8) == 0b11000000_u8);
    i8 y = 0b1111_i8 /* NOLINT(readability-magic-numbers) */;
    CHECK((y >>= 3_i8) == 0b1_i8);
    y = 0b1111_i8 /* NOLINT(readability-magic-numbers) */;
    CHECK((y <<= 2_i8) == 0b111100_i8);
    y = 0b1111_i8 /* NOLINT(readability-magic-numbers) */;
    CHECK((y >>= -1_i8) == 0b11110_i8);
    y = 0b1111_i8 /* NOLINT(readability-magic-numbers) */;
    CHECK((y <<= -2_i8) == 0b11_i8);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
