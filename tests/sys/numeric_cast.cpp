#include <cstdint>

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>

TEST_CASE("Casting within range preserves value.", "[sys][numeric_cast][unsafe]")
{
    CHECK(sys::bnumeric_cast<int32_t>(10, unsafe()) == 10);
    CHECK(sys::bnumeric_cast<uint8_t>(255, unsafe()) == 255);
    CHECK(sys::bnumeric_cast<int64_t>(-42, unsafe()) == -42);
}
TEST_CASE("Casting above range saturates to max.", "[sys][numeric_cast][unsafe]")
{
    CHECK(sys::bnumeric_cast<uint8_t>(300, unsafe()) == 255);
    CHECK(sys::bnumeric_cast<int8_t>(128, unsafe()) == 127);
    CHECK(sys::bnumeric_cast<uint16_t>(100000, unsafe()) == 65535);
}
TEST_CASE("Casting below range saturates to min.", "[sys][numeric_cast][unsafe]")
{
    CHECK(sys::bnumeric_cast<uint8_t>(-1, unsafe()) == 0);
    CHECK(sys::bnumeric_cast<int8_t>(-129, unsafe()) == -128);
    CHECK(sys::bnumeric_cast<uint32_t>(-500, unsafe()) == 0);
}

TEST_CASE("Casting within range returns valid result.", "[sys][numeric_cast]")
{
    CHECK(sys::bnumeric_cast<int32_t>(10).expect() == 10);   // NOLINT(readability-magic-numbers)
    CHECK(sys::bnumeric_cast<uint8_t>(255).expect() == 255); // NOLINT(readability-magic-numbers)
}
TEST_CASE("Casting out of range returns empty result.", "[sys][numeric_cast]")
{
    CHECK_FALSE(sys::bnumeric_cast<uint8_t>(300)); // NOLINT(readability-magic-numbers)
    CHECK_FALSE(sys::bnumeric_cast<uint8_t>(-1));  // NOLINT(readability-magic-numbers)
    CHECK_FALSE(sys::bnumeric_cast<int8_t>(128));  // NOLINT(readability-magic-numbers)
}

// NOLINTEND(misc-include-cleaner)
