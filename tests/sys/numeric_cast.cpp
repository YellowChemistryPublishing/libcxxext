// NOLINTBEGIN(misc-include-cleaner)

#include <cstdint>

#include <catch2/catch_all.hpp>

#include <module/sys>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Saturating casts work as expected. | `sys::numeric_cast(..., unsafe())`")
{
    SECTION("Casting within range preserves value.")
    {
        CHECK(sys::numeric_cast<int32_t>(10, unsafe()) == 10);
        CHECK(sys::numeric_cast<uint8_t>(255, unsafe()) == 255);
        CHECK(sys::numeric_cast<int64_t>(-42, unsafe()) == -42);
    }

    SECTION("Casting above range saturates to max.")
    {
        CHECK(sys::numeric_cast<uint8_t>(300, unsafe()) == 255);
        CHECK(sys::numeric_cast<int8_t>(128, unsafe()) == 127);
        CHECK(sys::numeric_cast<uint16_t>(100000, unsafe()) == 65535);
    }

    SECTION("Casting below range saturates to min.")
    {
        CHECK(sys::numeric_cast<uint8_t>(-1, unsafe()) == 0);
        CHECK(sys::numeric_cast<int8_t>(-129, unsafe()) == -128);
        CHECK(sys::numeric_cast<uint32_t>(-500, unsafe()) == 0);
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Checking casts work as expected. | `sys::numeric_cast(...)`")
{
    SECTION("Casting within range returns valid result.")
    {
        auto res1 = sys::numeric_cast<int32_t>(10); // NOLINT(readability-magic-numbers)
        CHECK(res1.expect() == 10);

        auto res2 = sys::numeric_cast<uint8_t>(255); // NOLINT(readability-magic-numbers)
        CHECK(res2.expect() == 255);
    }

    SECTION("Casting out of range returns empty result.")
    {
        auto res1 = sys::numeric_cast<uint8_t>(300); // NOLINT(readability-magic-numbers)
        CHECK_FALSE(res1);

        auto res2 = sys::numeric_cast<uint8_t>(-1); // NOLINT(readability-magic-numbers)
        CHECK_FALSE(res2);

        auto res3 = sys::numeric_cast<int8_t>(128); // NOLINT(readability-magic-numbers)
        CHECK_FALSE(res3);
    }
}

// NOLINTEND(misc-include-cleaner)
