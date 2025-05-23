#include <print>

#include <module/sys.Threading.hpp>
#include <module/sys.hpp>

using namespace sys;

Task<> doSomeWork(char taskID, i32 delayTime)
{
    std::println("(In {}.) Doing some work...", taskID);
    co_await Task<>::delay(delayTime);
    std::println("(In {}.) Work done!", taskID);
}
__async(void) parallelRoutine(char taskID, i32 delayTime)
{
    for (int i = 0; i < 8; i++)
        co_await doSomeWork(taskID, delayTime);
}

int main()
{
    __thread_pool pool;
    parallelRoutine('a', 1000);
    parallelRoutine('b', 10);
}
