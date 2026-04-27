#include <cstdint>
#include <limits>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("Casting within range preserves value.", "[sys][bnumeric_cast][unsafe]")
{
    CHECK(sys::bnumeric_cast<int32_t>(10, unsafe) == 10);
    CHECK(sys::bnumeric_cast<uint8_t>(255, unsafe) == 255);
    CHECK(sys::bnumeric_cast<int64_t>(-42, unsafe) == -42);
    CHECK(sys::bnumeric_cast<uint16_t>(65535.0f, unsafe) == 65535); // NOLINT(readability-magic-numbers)
    CHECK(sys::bnumeric_cast<int16_t>(-10.5f, unsafe) == -10);      // NOLINT(readability-magic-numbers)
}
TEST_CASE("Casting above range saturates to max.", "[sys][bnumeric_cast][unsafe]")
{
    CHECK(sys::bnumeric_cast<uint8_t>(300, unsafe) == 255);
    CHECK(sys::bnumeric_cast<int8_t>(128, unsafe) == 127);
    CHECK(sys::bnumeric_cast<uint16_t>(100000, unsafe) == 65535);
    CHECK(sys::bnumeric_cast<uint16_t>(65535.1f, unsafe) == 65535); // NOLINT(readability-magic-numbers)
    CHECK(sys::bnumeric_cast<uint64_t>(std::numeric_limits<float>::infinity(), unsafe) == std::numeric_limits<uint64_t>::max());
    CHECK(sys::bnumeric_cast<uint64_t>(std::numeric_limits<float>::quiet_NaN(), unsafe) == sys::bsentinel<uint64_t>());
}
TEST_CASE("Casting below range saturates to min.", "[sys][bnumeric_cast][unsafe]")
{
    CHECK(sys::bnumeric_cast<uint8_t>(-1, unsafe) == 0);
    CHECK(sys::bnumeric_cast<int8_t>(-129, unsafe) == -128);
    CHECK(sys::bnumeric_cast<uint32_t>(-500, unsafe) == 0);
    CHECK(sys::bnumeric_cast<int16_t>(-10000000.0f, unsafe) == -32768); // NOLINT(readability-magic-numbers)
    CHECK(sys::bnumeric_cast<int32_t>(-std::numeric_limits<float>::infinity(), unsafe) == std::numeric_limits<int32_t>::lowest());
}

TEST_CASE("Casting within range returns valid result.", "[sys][bnumeric_cast]")
{
    CHECK(sys::bnumeric_cast<int32_t>(10).expect() == 10);   // NOLINT(readability-magic-numbers)
    CHECK(sys::bnumeric_cast<uint8_t>(255).expect() == 255); // NOLINT(readability-magic-numbers)
    CHECK(sys::bnumeric_cast<int64_t>(-42).expect() == -42);
    CHECK(sys::bnumeric_cast<uint16_t>(65535.0f).expect() == 65535); // NOLINT(readability-magic-numbers)
    CHECK(sys::bnumeric_cast<int16_t>(-10.5f).expect() == -10);      // NOLINT(readability-magic-numbers)
}
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Casting out of range returns empty result.", "[sys][bnumeric_cast]")
{
    sys::bnumeric_cast<uint8_t>(300).expect_err();       // NOLINT(readability-magic-numbers)
    sys::bnumeric_cast<uint8_t>(-1).expect_err();        // NOLINT(readability-magic-numbers)
    sys::bnumeric_cast<int8_t>(128).expect_err();        // NOLINT(readability-magic-numbers)
    sys::bnumeric_cast<uint16_t>(100000).expect_err();   // NOLINT(readability-magic-numbers)
    sys::bnumeric_cast<uint16_t>(65535.1f).expect_err(); // NOLINT(readability-magic-numbers)
    sys::bnumeric_cast<uint64_t>(std::numeric_limits<float>::infinity()).expect_err();
    sys::bnumeric_cast<uint64_t>(std::numeric_limits<float>::quiet_NaN()).expect_err();
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
