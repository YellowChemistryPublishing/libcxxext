// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("meta::generic_nullable_adaptor<...>", "[sys][meta][generic_nullable_adaptor]")
{
    CHECK(sys::meta::generic_nullable_adaptor(nullptr).is_null());
    CHECK(sys::meta::generic_nullable_adaptor(_as(nullptr, int*)).is_null());

    {
        struct
        {
            bool operator!() const noexcept { return false; }
        } nullable;
        CHECK_FALSE(sys::meta::generic_nullable_adaptor(nullable).is_null());
    }

    {
        struct
        {
            bool operator==(std::nullptr_t) const noexcept { return false; }
        } nullable;
        CHECK_FALSE(sys::meta::generic_nullable_adaptor(nullable).is_null());
    }
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
