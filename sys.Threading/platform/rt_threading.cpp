#include "rt_threading.h"
#include "rt.h"

#include <atomic>
#include <coroutine>
#include <cstddef>
#include <new> // NOLINT(misc-include-cleaner)

using namespace sys::platform;

std::atomic<thread_pool*> thread_pool::instance;

extern "C" void* ::sys::platform::_task_operator_new(size_t sz)
{
    return ::operator new(sz); // NOLINT(misc-include-cleaner)
}
extern "C" void ::sys::platform::_task_operator_delete(void* ptr)
{
    ::operator delete(ptr); // NOLINT(misc-include-cleaner)
}

extern "C" void ::sys::platform::_launch_async(void* addr)
{
    if (auto* threadPool = thread_pool::instance.load())
        threadPool->push(std::coroutine_handle<>::from_address(addr));
}
