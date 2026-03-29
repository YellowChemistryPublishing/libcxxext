#include <exception>
#include <memory>

#include <module/sys>

static_assert(sys::INullablePointer<void*>);
#if _libcxxext_compiler_msvc // Because this doesn't actually satisfy `sys::INothrowSwappable` on libstdc++.
static_assert(sys::INullablePointer<std::exception_ptr>);
#endif
static_assert(sys::INullablePointer<std::unique_ptr<int>::pointer>);
