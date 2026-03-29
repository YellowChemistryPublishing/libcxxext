#include <exception>
#include <memory>

#include <module/sys>

static_assert(sys::INullablePointer<void*>);
static_assert(sys::INullablePointer<std::exception_ptr>);
static_assert(sys::INullablePointer<std::unique_ptr<int>::pointer>);
