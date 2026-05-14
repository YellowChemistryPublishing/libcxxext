// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("destructor::destructor(), optional_destructor::optional_destructor(), destructor::~destructor(), optional_destructor::~optional_destructor()",
          "[sys][destructor][optional_destructor]")
{
    sz called = 0_uz;
    {
        const sys::destructor d1 = [&called]() noexcept -> void { called += 1_uz; };
        const sys::optional_destructor d2 = [&called]() noexcept -> void { called += 1_uz; };
        CHECK(called == 0_uz);
    }
    CHECK(called == 2_uz);
}

TEST_CASE("optional_destructor::optional_destructor(optional_destructor&&), optional_destructor::operator=(optional_destructor&&)", "[sys][optional_destructor]")
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

        _nowarn_begin_one_gcc("-Wself-move");
        _nowarn_begin_one_clang(_clwarn_clang_self_move);
        d1 = std::move(d1);
        d2 = std::move(d2); // NOLINT(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
        _nowarn_end_clang();
        _nowarn_end_gcc();
    }

    CHECK(called == 2_uz);
}

TEST_CASE("optional_destructor::clear()", "[sys][optional_destructor]")
{
    static sz called = 0_uz;

    {
        sys::optional_destructor d = []() noexcept -> void { called += 1_uz; };
        d.clear();
    }

    {
        using functor = decltype([]() noexcept -> void { called += 1_uz; });
        sys::optional_destructor d1 = functor();
        d1.clear();
        const sys::optional_destructor d2 = std::move(d1);
    }

    {
        using functor = decltype([]() noexcept -> void { called += 1_uz; });
        sys::optional_destructor d1 = functor();
        sys::optional_destructor d2 = functor();
        d2.clear();
        d1 = std::move(d2);
    }

    CHECK(called == 0_uz);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
