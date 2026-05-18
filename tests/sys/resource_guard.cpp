// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("resource_guard<...>::~resource_guard()", "[sys][resource_guard]")
{
    i32 counter = 1_i32;
    {
        constexpr void (*releaseFn)(i32&) noexcept = +[](i32& counter) noexcept -> void { counter -= 1_i32; };
        const sys::resource_guard<i32, releaseFn> guard(counter, unsafe);
    }
    CHECK(counter == 0_i32);
}

TEST_CASE("resource_guard<...>::resource_guard(resource_guard&&), resource_guard::operator=(resource_guard&&), swap(resource_guard&, resource_guard&)", "[sys][resource_guard]")
{
    i32 counter = 1_i32;
    constexpr auto releaseFn = +[](i32& counter) noexcept -> void { counter -= 1_i32; };
    sys::resource_guard<i32, releaseFn> guardFrom(counter, unsafe);
    {
        sys::resource_guard<i32, releaseFn> guard(std::move(guardFrom));
        guardFrom = std::move(guard);
        swap(guard, guardFrom);

        CHECK(guard);
        CHECK_FALSE(guardFrom);
        // `guard` calls `releaseFn`.
    }

    CHECK(counter == 0_i32);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
