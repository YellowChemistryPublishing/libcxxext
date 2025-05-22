#pragma once

#include <thread>

#include <LanguageSupport.h>

struct __tcs_isr
{
    _inline_always __tcs_isr()
    { }
    _inline_always ~__tcs_isr()
    { }
};
struct __tcs
{
    _inline_always __tcs()
    { }
    _inline_always ~__tcs()
    { }
};

using __thread_id = std::thread::id;

class __thread_type
{
    void* handle;

    inline __thread_type(void* handle) : handle(handle)
    { }
public:
    [[noreturn]] inline static __thread_type currentThread()
    { }

    [[noreturn]] inline __thread_id id()
    { }
};

#define __task_yield()                      \
    inline static Task<void> yield()        \
    requires (std::is_same<T, void>::value) \
    {                                       \
        co_return;                          \
    }
#define __task_delay()                          \
    inline static Task<void> delay(uint32_t ms) \
    requires (std::is_same<T, void>::value)     \
    {                                           \
        co_return;                              \
    }
#define __task_yield_and_resume()
#define __task_yield_and_continue()

constexpr u32 __task_max_delay = std::numeric_limits<u32>::max();
