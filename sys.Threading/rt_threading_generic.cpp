#include "rt.h"

#include <chrono>

std::atomic<__thread_pool*> __thread_pool::instance;

extern "C" void* __task_operator_new(size_t sz)
{
    return ::operator new(sz);
}
extern "C" void __task_operator_delete(void* ptr)
{
    ::operator delete(ptr);
}

extern "C" void __launch_async(void* addr)
{
    if (auto* threadPool = __thread_pool::instance.load())
        threadPool->queue.enqueue(std::coroutine_handle<>::from_address(addr));
}
