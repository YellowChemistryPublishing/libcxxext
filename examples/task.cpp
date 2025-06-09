#if defined(_MSC_VER) && _MSC_VER
#pragma warning(disable : _clWarn_unreachable, justification : "Erroneously generated for compiler coroutine codegen.")
#endif

#include <print>

#include <module/sys.Threading>
#include <module/sys>

using namespace sys;

task<> doSomeWork(char taskID, i32 delayTime)
{
    std::println("(In {}.) Doing some work...", taskID);
    co_await task<>::delay(delayTime);
    std::println("(In {}.) Work done!", taskID);
}
async parallelRoutine(char taskID, i32 delayTime)
{
    for (int i = 0; i < 8; i++) co_await doSomeWork(taskID, delayTime);
}

int main()
{
    __thread_pool pool;
    parallelRoutine('a', 1000);
    parallelRoutine('b', 10);
}
