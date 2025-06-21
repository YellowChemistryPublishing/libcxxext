#include <print>

#include <module/sys.Threading>
#include <module/sys>

_push_nowarn_msvc(_clWarn_msvc_unreachable) // Erroneously generated for compiler coroutine codegen.

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
    ::platform::ThreadPool pool;
    parallelRoutine('a', 1000);
    parallelRoutine('b', 10);
}
