#include <memory>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>

TEST_CASE("Invariably passes.", "[sys][traits][concepts]") { CHECK(true); }

static_assert(sys::INullablePointer<void*>);
#if _libcxxext_compiler_msvc // Because this doesn't actually satisfy `sys::INothrowSwappable` on libstdc++.
static_assert(sys::INullablePointer<std::exception_ptr>);
#endif
static_assert(sys::INullablePointer<std::unique_ptr<int>::pointer>);

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
