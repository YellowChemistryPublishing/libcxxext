#include <memory>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("sys::INullablePointer<...>", "[sys][INullablePointer]")
{
    STATIC_CHECK(sys::INullablePointer<void*>);
#if _libcxxext_compiler_msvc // Because this doesn't actually satisfy `sys::INothrowSwappable` on libstdc++.
    STATIC_CHECK(sys::INullablePointer<std::exception_ptr>);
#endif
    STATIC_CHECK(sys::INullablePointer<std::unique_ptr<int>::pointer>);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
