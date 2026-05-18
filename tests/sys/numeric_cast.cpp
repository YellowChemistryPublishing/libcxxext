#include <cstdint>
#include <limits>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

// NOLINTBEGIN(readability-magic-numbers)

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("bnumeric_cast<...>(..., decltype(unsafe))", "[sys][bnumeric_cast][unsafe]", int8_t, int16_t, int32_t, uint8_t,
                                                                     uint16_t, uint32_t)
{
    CHECK(sys::bnumeric_cast<int32_t>(10, unsafe) == 10);
    CHECK(sys::bnumeric_cast<uint8_t>(255l, unsafe) == 255);
    CHECK(sys::bnumeric_cast<int64_t>(-42ll, unsafe) == -42);
    CHECK(sys::bnumeric_cast<uint16_t>(65535.0f, unsafe) == 65535);
    CHECK(sys::bnumeric_cast<int16_t>(-10.5f, unsafe) == -10);

    CHECK(sys::bnumeric_cast<uint8_t>(300, unsafe) == 255);
    CHECK(sys::bnumeric_cast<int8_t>(128, unsafe) == 127);
    CHECK(sys::bnumeric_cast<uint16_t>(100000, unsafe) == 65535);
    CHECK(sys::bnumeric_cast<uint16_t>(65535.1f, unsafe) == 65535);
    CHECK(sys::bnumeric_cast<TestType>(std::numeric_limits<float>::infinity(), unsafe) == std::numeric_limits<TestType>::max());

    CHECK(sys::bnumeric_cast<uint8_t>(-1, unsafe) == 0);
    CHECK(sys::bnumeric_cast<int8_t>(-129, unsafe) == -128);
    CHECK(sys::bnumeric_cast<uint32_t>(-500, unsafe) == 0);
    CHECK(sys::bnumeric_cast<TestType>(-1000000000000000000.0f, unsafe) == std::numeric_limits<TestType>::lowest());
    CHECK(sys::bnumeric_cast<TestType>(-std::numeric_limits<float>::infinity(), unsafe) == std::numeric_limits<TestType>::lowest());

    CHECK(sys::bnumeric_cast<TestType>(std::numeric_limits<float>::quiet_NaN(), unsafe) == sys::bsentinel<TestType>());
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("bnumeric_cast<...>(...)", "[sys][bnumeric_cast]", int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t)
{
    CHECK(sys::bnumeric_cast<int32_t>(10).expect() == 10);
    CHECK(sys::bnumeric_cast<uint8_t>(255).expect() == 255);
    CHECK(sys::bnumeric_cast<int64_t>(-42).expect() == -42);
    CHECK(sys::bnumeric_cast<uint16_t>(65535.0f).expect() == 65535);
    CHECK(sys::bnumeric_cast<int16_t>(-10.5f).expect() == -10);

    CHECK(sys::bnumeric_cast<uint8_t>(300).operator!());
    CHECK(sys::bnumeric_cast<uint8_t>(-1).operator!());
    CHECK(sys::bnumeric_cast<int8_t>(128).operator!());
    CHECK(sys::bnumeric_cast<uint16_t>(100000).operator!());
    CHECK(sys::bnumeric_cast<uint16_t>(65535.1f).operator!());
    CHECK(sys::bnumeric_cast<TestType>(std::numeric_limits<float>::infinity()).operator!());
    CHECK(sys::bnumeric_cast<TestType>(-std::numeric_limits<double>::infinity()).operator!());
    CHECK(sys::bnumeric_cast<TestType>(std::numeric_limits<long double>::quiet_NaN()).operator!());
}

// NOLINTEND(readability-magic-numbers)
// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
