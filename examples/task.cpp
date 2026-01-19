#include <new>
#include <print>

// NOLINTBEGIN(misc-include-cleaner)
// NOLINTBEGIN(llvm-include-order): Alas clang-format and clang-tidy disagree.

#include <module/sys.Threading>
#include <module/sys>

// NOLINTEND(llvm-include-order)

_push_nowarn_msvc(_clwarn_msvc_unreachable); // Erroneously generated for compiler coroutine codegen.

using namespace sys;

static task<> do_some_work(char taskID, i32 delayTime)
{
    std::println("(In {}.) Doing some work...", taskID);
    co_await task<>::delay(delayTime);
    std::println("(In {}.) Work done!", taskID);
}
// NOLINTNEXTLINE(readability-static-accessed-through-instance)
static async parallel_routine(char taskID, i32 delayTime)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (int i = 0; i < 8; i++)
        co_await do_some_work(taskID, delayTime);
}

int main()
{
    try
    {
        const ::platform::thread_pool pool;
        parallel_routine('a', 1000_i32); // NOLINT(readability-magic-numbers)
        parallel_routine('b', 10_i32);   // NOLINT(readability-magic-numbers)
    }
    catch (const std::bad_alloc&)
    {
        return _as(int, 0xDEADBEEF);
    }
}

// NOLINTEND(misc-include-cleaner)
