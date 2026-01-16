#include <new>
#include <print>

// NOLINTBEGIN(misc-include-cleaner)
// NOLINTBEGIN(llvm-include-order): Alas clang-format and clang-tidy disagree.

#include <module/sys.Threading>
#include <module/sys>

// NOLINTEND(llvm-include-order)

_push_nowarn_msvc(_clWarn_msvc_unreachable); // Erroneously generated for compiler coroutine codegen.

using namespace sys;

static task<> doSomeWork(char taskID, i32 delayTime)
{
    std::println("(In {}.) Doing some work...", taskID);
    co_await task<>::delay(delayTime);
    std::println("(In {}.) Work done!", taskID);
}
// NOLINTNEXTLINE(readability-static-accessed-through-instance)
static async parallelRoutine(char taskID, i32 delayTime)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (int i = 0; i < 8; i++) co_await doSomeWork(taskID, delayTime);
}

int main()
{
    try
    {
        const ::platform::thread_pool pool;
        parallelRoutine('a', 1000); // NOLINT(readability-magic-numbers)
        parallelRoutine('b', 10);   // NOLINT(readability-magic-numbers)
    }
    catch (const std::bad_alloc&)
    {
        return _as(int, 0xDEADBEEF);
    }
}

// NOLINTEND(misc-include-cleaner)
