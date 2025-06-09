#pragma once

#include <chrono>
#include <coroutine>
#include <mutex>
#include <print>
#include <queue>
#include <stop_token>
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

    inline __thread_type(std::nullptr_t)
    { }
public:
    inline static const __thread_type currentThread()
    {
        return nullptr;
    }
    inline static void yield()
    {
        std::this_thread::yield();
    }
    inline static void sleep(i32 ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(+ms));
    }

    template <typename Func, typename... Args>
    requires (sys::IFunc<Func, void(Args...)>)
    constexpr __thread_type(Func&& func, Args&&... args)
    {
        this->handle = std::thread([](Func func, Args... args) { func(std::forward<Args>(args)...); }, std::forward<Func>(func), std::forward<Args>(args)...);
    }
    inline __thread_type(__thread_type&& other) noexcept
    {
        swap(*this, other);
    }
    inline ~__thread_type()
    {
        this->join();
    }

    inline bool isAlive()
    {
        return this->handle.joinable() || this->handle.get_id() == __thread_id();
    }
    inline __thread_id id() const noexcept
    {
        __thread_id id = this->handle.get_id();
        return id == __thread_id() ? std::this_thread::get_id() : id;
    }

    inline void join()
    {
        if (this->handle.joinable())
            this->handle.join();
    }

    friend inline void swap(__thread_type& a, __thread_type& b)
    {
        using std::swap;
        swap(a.handle, b.handle);
    }
};

struct __concurrent_queue
{
    std::queue<std::coroutine_handle<>> queue;
    std::mutex qlock;

    inline void enqueue(std::coroutine_handle<> handle)
    {
        std::lock_guard lock(this->qlock);
        this->queue.push(handle);
    }
    inline bool try_dequeue(std::coroutine_handle<>& handle)
    {
        std::lock_guard lock(this->qlock);
        if (this->queue.empty())
            return false;
        handle = this->queue.front();
        this->queue.pop();
        return true;
    }
};

struct __thread_pool
{
    static std::atomic<__thread_pool*> instance;

    std::vector<std::jthread> threads;
    __concurrent_queue queue;

    inline __thread_pool()
    {
        __thread_pool::instance = this;
        this->threads.emplace_back([this](std::stop_token token)
        {
            std::coroutine_handle<> handle;
            while (!token.stop_requested())
            {
                if (this->queue.try_dequeue(handle))
                    handle.resume();
                std::this_thread::yield();
            }
            while (this->queue.try_dequeue(handle))
            {
                handle.resume();
                std::this_thread::yield();
            }
        });
    }
    inline ~__thread_pool()
    {
        for (auto& thread : __thread_pool::threads) thread.request_stop();
        for (auto& thread : __thread_pool::threads) thread.join();
        __thread_pool::instance = nullptr;
    }
};

#define __task_yield()                      \
    inline static task<void> yield()        \
    requires (std::is_same<T, void>::value) \
    {                                       \
        co_return;                          \
    }
#define __task_delay()                                                                  \
    inline static task<void> delay(i32 ms)                                              \
    requires (std::is_same<T, void>::value)                                             \
    {                                                                                   \
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(+ms); \
        while (std::chrono::steady_clock::now() < until) co_await task<>::yield();      \
    }
#define __task_yield_and_resume()                          \
    if (auto* threadPool = __thread_pool::instance.load()) \
    threadPool->queue.enqueue(this->handle)
#define __task_yield_and_continue()                                \
    if (auto* threadPool = __thread_pool::instance.load())         \
    threadPool->queue.enqueue(this->handle.promise().continuation)

constexpr i32 __task_max_delay = std::numeric_limits<i32::Underlying>::max();
