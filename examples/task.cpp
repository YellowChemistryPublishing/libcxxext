#include <new>
#include <print>

#include <module/sys>
#include <module/sys.Threading>

_push_nowarn_msvc(_clwarn_msvc_unreachable); // Erroneously generated for compiler coroutine codegen.

static sys::task<> do_some_work(char taskID, i32 delayTime)
{
    std::println("(In {}.) Doing some work...", taskID);
    co_await sys::task<>::delay(delayTime);
    std::println("(In {}.) Work done!", taskID);
}
// NOLINTNEXTLINE(readability-static-accessed-through-instance)
static sys::async parallel_routine(char taskID, i32 delayTime)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (int i = 0; i < 8; i++)
        co_await do_some_work(taskID, delayTime);
}

int main()
{
    try
    {
        const sys::platform::thread_pool pool;
        parallel_routine('a', 1000_i32); // NOLINT(readability-magic-numbers)
        parallel_routine('b', 10_i32);   // NOLINT(readability-magic-numbers)
    }
    catch (const std::bad_alloc&)
    {
        return _as(int, 0xDEADBEEF);
    }
}
