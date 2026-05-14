#include <utility> // NOLINT(misc-include-cleaner): Remove for an actual test, obviously.
// Add more standard headers as needed...

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
// Add more module headers as needed...

TEST_CASE("function name here, ...", "[sys][submodulenamehere][typenamehere]")
{
    REQUIRE(true);
    // Although, we prefer `CHECK(...)` unless something must hold for a test to continue.
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
