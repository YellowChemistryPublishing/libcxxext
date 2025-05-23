#pragma once

#include <chrono>
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
    std::thread handle;

    constexpr __thread_type(std::nullptr_t)
    { }
public:
    constexpr static const __thread_type currentThread()
    {
        return nullptr;
    }
    constexpr static void yield()
    {
        std::this_thread::yield();
    }
    constexpr static void sleep(i32 ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    template <typename Func, typename... Args>
    requires (sys::IFunc<Func, void(Args...)>)
    constexpr __thread_type(Func&& func, Args&&... args)
    {
        this->handle = std::thread([](Func func, Args... args) { func(std::forward<Args>(args)...); }, std::forward<Func>(func), std::forward<Args>(args)...);
    }
    constexpr __thread_type(__thread_type&& other) noexcept
    {
        swap(*this, other);
    }
    constexpr ~__thread_type()
    {
        this->join();
    }

    constexpr bool isAlive()
    {
        return this->handle.joinable() || this->handle.get_id() == __thread_id();
    }
    constexpr __thread_id id() const noexcept
    {
        __thread_id id = this->handle.get_id();
        return id == __thread_id() ? std::this_thread::get_id() : id;
    }

    constexpr void join()
    {
        if (this->handle.joinable())
            this->handle.join();
    }

    friend constexpr void swap(__thread_type& a, __thread_type& b)
    {
        using std::swap;
        swap(a.handle, b.handle);
    }
};

#define __task_yield()                      \
    inline static Task<void> yield()        \
    requires (std::is_same<T, void>::value) \
    {                                       \
        co_return;                          \
    }
#define __task_delay()                      \
    inline static Task<void> delay(u32 ms)  \
    requires (std::is_same<T, void>::value) \
    {                                       \
        co_return;                          \
    }
#define __task_yield_and_resume()
#define __task_yield_and_continue()

constexpr i32 __task_max_delay = std::numeric_limits<i32::UnderlyingType>::max();
