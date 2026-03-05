#pragma once

/// @file rt_threading.h

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

#include <LanguageSupport.h>
#include <Numeric.h>
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

    /// @internal
    /// @brief Thread handle.
    class thread_handle
    {
        std::thread handle;

        /* NOLINT(hicpp-explicit-conversions) */ thread_handle(std::nullptr_t) { }
    public:
        /// @brief Obtain a thread handle representing the current thread.
        static thread_handle current_thread() { return nullptr; }
        /// @brief Yield the current thread to the scheduler.
        static void yield() { std::this_thread::yield(); }
        /// @brief Sleep for `ms`.
        static void sleep(i32 ms) { std::this_thread::sleep_for(std::chrono::milliseconds(*ms)); }

        /// @brief Spin off a thread to execution `func` with `args...`.
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

        /// @brief Whether thread is running.
        bool is_alive() { return this->handle.joinable() || this->handle.get_id() == thread_id(); }
        /// @brief Numerical identifier for thread.
        [[nodiscard]] thread_id id() const noexcept
        {
            const thread_id id = this->handle.get_id();
            return id == thread_id() ? std::this_thread::get_id() : id;
        }

        /// @brief Block until thread handle completes.
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

    /// @internal
    /// @brief Thread-safe coroutine queue.
    struct concurrent_queue
    {
        /// @brief Push `handle` to the queue.
        void enqueue(std::coroutine_handle<> handle)
        {
            const std::unique_lock lock(this->qlock);
            this->queue.push(handle);
            this->cv.notify_one();
        }
        /// @brief Dequeue into `handle` if queue non-empty.
        /// @warning `unsafe` because unguarded.
        bool try_dequeue(std::coroutine_handle<>& handle, unsafe)
        {
            if (this->queue.empty())
                return false;

            handle = this->queue.front();
            this->queue.pop();
            return true;
        }
        /// @copybrief sys::platform::concurrent_queue::try_dequeue(std::coroutine_handle<>&,unsafe)
        /// @return Whether queue had element to dequeue.
        bool try_dequeue(std::coroutine_handle<>& handle)
        {
            const std::unique_lock lock(this->qlock);
            return this->try_dequeue(handle, unsafe());
        }
        /// @brief Dequeue into `handle` once queue is non-empty.
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

    /// @internal
    /// @brief Thread pool.
    struct thread_pool
    {
        static std::atomic<thread_pool*> instance; ///< Thread pool singleton.

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

        /// @brief Push coroutine to queue.
        void push(std::coroutine_handle<> handle) { this->queue.enqueue(handle); }
    private:
        std::vector<std::jthread> threads;
        concurrent_queue queue;
    };

    constexpr i32 task_max_delay = i32::highest(); ///< Platform-specific max supported delay duration.
} // namespace sys::platform

/// @def _impl_task_yield_rtype
/// @brief `sys::task<void>::yield()` return type.
#define _impl_task_yield_rtype ::sys::task<void>
/// @def _impl_task_yield()
/// @brief `sys::task<void>::yield()` implementation.
#define _impl_task_yield() co_return

/// @def _impl_task_delay_rtype
/// @brief `sys::task<void>::delay(...)` return type.
#define _impl_task_delay_rtype ::sys::task<void>
/// @def _impl_task_delay()
/// @brief `sys::task<void>::delay(...)` implementation.
#define _impl_task_delay()                                                          \
    auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(*ms); \
    while (std::chrono::steady_clock::now() < until)                                \
        co_await task<>::yield();

/// @def _task_yield_and_resume()
/// @brief Yield and resume implementation on suspend.
#define _task_yield_and_resume()                                          \
    if (auto* threadPool = ::sys::platform::thread_pool::instance.load()) \
    threadPool->push(this->handle)

/// @def _task_yield_and_continue()
/// @brief Yield and continue implementation of final suspend.
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
