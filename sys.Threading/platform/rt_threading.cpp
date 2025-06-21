#include "rt_threading.h"
#include "rt.h"

#include <atomic>
#include <coroutine>

using namespace sys::platform;

std::atomic<ThreadPool*> ThreadPool::instance;

extern "C" void* _task_operator_new(size_t sz)
{
    return ::operator new(sz);
}
extern "C" void _task_operator_delete(void* ptr)
{
    ::operator delete(ptr);
}

extern "C" void _launch_async(void* addr)
{
    if (auto* threadPool = ThreadPool::instance.load())
        threadPool->queue.enqueue(std::coroutine_handle<>::from_address(addr));
}
