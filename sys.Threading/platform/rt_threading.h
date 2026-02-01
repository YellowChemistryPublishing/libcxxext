#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <cstddef>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

#include <Integer.h>
#include <LanguageSupport.h>
#include <Traits.h>

namespace sys::platform
{
    struct noop_thread_critical_section_isr
    { };
    struct noop_thread_critical_section
    { };

    using thread_critical_section_isr = noop_thread_critical_section_isr;
    using thread_critical_section = noop_thread_critical_section;

    using thread_id = std::thread::id;

    class thread_handle
    {
        std::thread handle;

        thread_handle(std::nullptr_t) // NOLINT(hicpp-explicit-conversions)
        { }
    public:
        static thread_handle current_thread() { return nullptr; }
        static void yield() { std::this_thread::yield(); }
        static void sleep(i32 ms) { std::this_thread::sleep_for(std::chrono::milliseconds(*ms)); }

        template <typename Func, typename... Args>
        requires (sys::IFunc<Func, void(Args...)>)
        constexpr explicit thread_handle(Func&& func, Args&&... args)
        {
            this->handle = std::thread([](Func func, Args... args) { func(std::forward<Args>(args)...); }, std::forward<Func>(func), std::forward<Args>(args)...);
        }
        thread_handle(const thread_handle& other) = delete;
        thread_handle(thread_handle&& other) noexcept { swap(*this, other); }
        ~thread_handle() { this->join(); }

        thread_handle& operator=(const thread_handle& other) = delete;
        thread_handle& operator=(thread_handle&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        bool is_alive() { return this->handle.joinable() || this->handle.get_id() == thread_id(); }
        [[nodiscard]] thread_id id() const noexcept
        {
            const thread_id id = this->handle.get_id();
            return id == thread_id() ? std::this_thread::get_id() : id;
        }

        void join()
        {
            if (this->handle.joinable())
                this->handle.join();
        }

        friend void swap(thread_handle& a, thread_handle& b) noexcept
        {
            using std::swap;
            swap(a.handle, b.handle);
        }
    };

    struct concurrent_queue
    {
        void enqueue(std::coroutine_handle<> handle)
        {
            const std::unique_lock lock(this->qlock);
            this->queue.push(handle);
            this->cv.notify_one();
        }
        bool try_dequeue(std::coroutine_handle<>& handle, unsafe)
        {
            if (this->queue.empty())
                return false;

            handle = this->queue.front();
            this->queue.pop();
            return true;
        }
        bool try_dequeue(std::coroutine_handle<>& handle)
        {
            const std::unique_lock lock(this->qlock);
            return this->try_dequeue(handle, unsafe());
        }
        template <typename Func>
        bool wait_dequeue(std::coroutine_handle<>& handle, const Func& stopCond)
        {
            std::unique_lock lock(this->qlock);
            this->cv.wait(lock, [this, &stopCond]() { return !this->queue.empty() || stopCond(); });
            return this->try_dequeue(handle, unsafe());
        }
    private:
        std::queue<std::coroutine_handle<>> queue;
        std::mutex qlock;
        std::condition_variable cv;
    };

    struct thread_pool
    {
        static std::atomic<thread_pool*> instance;

        thread_pool()
        {
            thread_pool::instance = this;
            this->threads.emplace_back([this](const std::stop_token& token)
            {
                std::coroutine_handle<> handle;
                while (!token.stop_requested())
                {
                    while (this->queue.wait_dequeue(handle, [&token]() { return token.stop_requested(); }))
                        handle.resume();
                }
                while (this->queue.try_dequeue(handle))
                    handle.resume();
            });
        }
        thread_pool(const thread_pool&) = delete;
        thread_pool(thread_pool&&) = delete;
        ~thread_pool()
        {
            for (auto& thread : this->threads)
                thread.request_stop();
            for (auto& thread : this->threads)
                thread.join();
            thread_pool::instance = nullptr;
        }

        thread_pool& operator=(const thread_pool&) = delete;
        thread_pool& operator=(thread_pool&&) = delete;

        void push(std::coroutine_handle<> handle) { this->queue.enqueue(handle); }
    private:
        std::vector<std::jthread> threads;
        concurrent_queue queue;
    };

    constexpr i32 task_max_delay = i32::highest();
} // namespace sys::platform

#define _impl_task_yield_rtype ::sys::task<void>
#define _impl_task_yield() co_return

#define _impl_task_delay_rtype ::sys::task<void>
#define _impl_task_delay()                                                          \
    auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(*ms); \
    while (std::chrono::steady_clock::now() < until)                                \
        co_await task<>::yield();

#define _task_yield_and_resume()                                          \
    if (auto* threadPool = ::sys::platform::thread_pool::instance.load()) \
    threadPool->push(this->handle)

#define _task_yield_and_continue()                                            \
    _retry:                                                                   \
    try                                                                       \
    {                                                                         \
        if (auto* threadPool = ::sys::platform::thread_pool::instance.load()) \
            threadPool->push(this->handle.promise().continuation);            \
    }                                                                         \
    catch (const std::bad_alloc&)                                             \
    {                                                                         \
        std::this_thread::yield();                                            \
        goto _retry;                                                          \
    }
