#pragma once

#include <chrono>
#include <coroutine>
#include <mutex>
#include <print>
#include <queue>
#include <stop_token>
#include <thread>

#include <LanguageSupport.h>

namespace sys::platform
{
    struct NoOpThreadCriticalSectionISR
    {
        _inline_always NoOpThreadCriticalSectionISR()
        { }
        _inline_always ~NoOpThreadCriticalSectionISR()
        { }
    };
    struct NoOpThreadCriticalSection
    {
        _inline_always NoOpThreadCriticalSection()
        { }
        _inline_always ~NoOpThreadCriticalSection()
        { }
    };

    using ThreadID = std::thread::id;

    class ThreadHandle
    {
        std::thread handle;

        inline ThreadHandle(std::nullptr_t)
        { }
    public:
        inline static const ThreadHandle currentThread()
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
        constexpr ThreadHandle(Func&& func, Args&&... args)
        {
            this->handle = std::thread([](Func func, Args... args) { func(std::forward<Args>(args)...); }, std::forward<Func>(func), std::forward<Args>(args)...);
        }
        inline ThreadHandle(ThreadHandle&& other) noexcept
        {
            swap(*this, other);
        }
        inline ~ThreadHandle()
        {
            this->join();
        }

        inline bool isAlive()
        {
            return this->handle.joinable() || this->handle.get_id() == ThreadID();
        }
        inline ThreadID id() const noexcept
        {
            ThreadID id = this->handle.get_id();
            return id == ThreadID() ? std::this_thread::get_id() : id;
        }

        inline void join()
        {
            if (this->handle.joinable())
                this->handle.join();
        }

        friend inline void swap(ThreadHandle& a, ThreadHandle& b)
        {
            using std::swap;
            swap(a.handle, b.handle);
        }
    };

    struct ConcurrentQueue
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

    struct ThreadPool
    {
        static std::atomic<ThreadPool*> instance;

        std::vector<std::jthread> threads;
        ConcurrentQueue queue;

        inline ThreadPool()
        {
            ThreadPool::instance = this;
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
        inline ~ThreadPool()
        {
            for (auto& thread : ThreadPool::threads) thread.request_stop();
            for (auto& thread : ThreadPool::threads) thread.join();
            ThreadPool::instance = nullptr;
        }
    };

#define _impl_task_yield()                  \
    inline static task<void> yield()        \
    requires (std::is_same<T, void>::value) \
    {                                       \
        co_return;                          \
    }
#define _impl_task_delay()                                                              \
    inline static task<void> delay(i32 ms)                                              \
    requires (std::is_same<T, void>::value)                                             \
    {                                                                                   \
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(+ms); \
        while (std::chrono::steady_clock::now() < until) co_await task<>::yield();      \
    }
#define _task_yield_and_resume()                        \
    if (auto* threadPool = ::sys::platform::ThreadPool::instance.load()) \
    threadPool->queue.enqueue(this->handle)
#define _task_yield_and_continue()                                 \
    if (auto* threadPool = ::sys::platform::ThreadPool::instance.load())            \
    threadPool->queue.enqueue(this->handle.promise().continuation)

    constexpr i32 _task_max_delay = std::numeric_limits<i32::underlying_type>::max();
} // namespace sys::platform
