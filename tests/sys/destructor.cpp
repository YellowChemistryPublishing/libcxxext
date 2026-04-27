// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("`sys::destructor` calls provided cleanup function on destruction.", "[sys][destructor]")
{
    sz called = 0_uz;
    {
        const sys::destructor d1 = [&called]() noexcept -> void { called += 1_uz; };
        const sys::optional_destructor d2 = [&called]() noexcept -> void { called += 1_uz; };
        CHECK(called == 0_uz);
    }
    CHECK(called == 2_uz);
}

TEST_CASE("`sys::optional_destructor` does nothing if cleared.", "[sys][optional_destructor]")
{
    bool called = false;
    {
        sys::optional_destructor d = [&called]() noexcept -> void { called = true; };
        d.clear();
    }
    CHECK_FALSE(called);
}

// NOLINTBEGIN(misc-const-correctness): Spurious.

TEST_CASE("`sys::optional_destructor` correctly transfers ownership on move.", "[sys][optional_destructor]")
{
    static sz called = 0_uz;
    {
        sys::optional_destructor d1 = []() noexcept -> void { called += 1_uz; };
        const sys::optional_destructor d2 = std::move(d1);
    }
    {
        using functor = decltype([]() noexcept -> void { called += 1_uz; });
        sys::optional_destructor d1 = functor();
        sys::optional_destructor d2 = functor();
        d1 = std::move(d2);
        d2.clear(); // NOLINT(bugprone-use-after-move)
    }
    CHECK(called == 2_uz);
}

TEST_CASE("`sys::optional_destructor` preserves cleared state on move.", "[sys][optional_destructor]")
{
    static sz called = 0_uz;
    using functor = decltype([]() noexcept -> void { called += 1_uz; });
    {
        sys::optional_destructor d1 = functor();
        d1.clear();
        const sys::optional_destructor d2 = std::move(d1);
    }
    {
        sys::optional_destructor d1 = functor();
        sys::optional_destructor d2 = functor();
        d2.clear();
        d1 = std::move(d2);
    }
    CHECK(called == 0_uz);
}

// NOLINTEND(misc-const-correctness)
// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
