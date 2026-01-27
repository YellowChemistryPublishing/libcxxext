#pragma once

#include <CompilerWarnings.h>
_push_nowarn_msvc(_clwarn_msvc_unreachable); // Erroneously generated for compiler coroutine codegen.

#include <concepts>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <utility>

#include <Integer.h>
#include <LanguageSupport.h>
#include <rt.h>
#include <rt_threading.h>

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)

namespace sys
{
    struct async;

    template <typename T>
    class task;
} // namespace sys

namespace sys::internal
{
    template <typename T>
    struct task_promise;

    /// @brief Promise for a `sys::async`.
    struct async_promise
    {
        constexpr async_promise() noexcept = default;
        async_promise(const async_promise&) = delete;
        async_promise(async_promise&&) = delete;
        _inline_always constexpr ~async_promise() = default;

        async_promise& operator=(const async_promise&) = delete;
        async_promise& operator=(async_promise&&) = delete;

        _inline_always async get_return_object();

        [[nodiscard]] _inline_always constexpr static std::suspend_always initial_suspend() noexcept { return {}; }
        [[nodiscard]] _inline_always constexpr static std::suspend_never final_suspend() noexcept { return {}; }

        static void unhandled_exception() noexcept { std::rethrow_exception(std::current_exception()); }
        _inline_always constexpr void return_void() const noexcept { }
    };

    /// @brief Awaiter for a task.
    ///
    /// @tparam T Coroutine return type.
    /// @note Pass `byref`.
    template <typename T>
    struct task_awaiter
    {
        std::coroutine_handle<task_promise<T>> handle;

        [[nodiscard]] _inline_always constexpr bool await_ready() const noexcept { return !this->handle || this->handle.done(); }
        _inline_never auto await_suspend(std::coroutine_handle<> parent)
        {
            this->handle.promise().continuation = parent;
            _task_yield_and_resume();
        }
        _inline_always constexpr T await_resume() const noexcept(std::is_same_v<T, void>)
        {
            if (this->handle.promise().exception) [[unlikely]]
                std::rethrow_exception(this->handle.promise().exception);
            if constexpr (!std::is_same_v<T, void>)
                return std::move(*reinterpret_cast<T*>(&this->handle.promise().value));
        }
    };
    /// @brief Final awaiter for a task.
    ///
    /// @tparam T Coroutine return type.
    /// @note Pass `byref`.
    template <typename T>
    struct task_final_awaiter
    {
        std::coroutine_handle<task_promise<T>> handle;

        [[nodiscard]] _inline_always constexpr bool await_ready() const noexcept { return false; }
        _inline_never auto await_suspend(std::coroutine_handle<>) noexcept { _task_yield_and_continue(); }
        constexpr void await_resume() const noexcept { }
    };

    template <typename T>
    struct task_promise_b
    {
        std::coroutine_handle<> continuation;
        std::exception_ptr exception = nullptr;

        static void* operator new(size_t sz) noexcept { return ::sys::platform::task_operator_new(sz); }
        static void operator delete(void* ptr) noexcept { ::sys::platform::task_operator_delete(ptr); }

        [[nodiscard]] _inline_always constexpr std::suspend_always initial_suspend() const noexcept { return {}; }

        _inline_always task<T> get_return_object() { return task<T>(std::coroutine_handle<task_promise<T>>::from_promise(*static_cast<task_promise<T>*>(this))); }

        [[noreturn]] static task<T> get_return_object_on_allocation_failure() { _throw(std::bad_alloc()); }
        _inline_always void unhandled_exception() { this->exception = std::current_exception(); }
    };
    /// @brief Promise for a `sys::task`.
    ///
    /// @tparam T Coroutine return type.
    /// @note Pass `byref`.
    template <typename T>
    struct task_promise final : public task_promise_b<T> // NOLINT(hicpp-member-init)
    {
        constexpr task_promise() noexcept { };
        constexpr task_promise(const task_promise&) = delete;
        constexpr task_promise(task_promise&&) = delete;
        ~task_promise() noexcept(noexcept(this->value.~T()))
        {
            if (this->has_value) [[likely]]
                this->value.~T();
        }

        constexpr task_promise& operator=(const task_promise&) = delete;
        constexpr task_promise& operator=(task_promise&&) = delete;

        _inline_always task_final_awaiter<T> final_suspend() noexcept { return task_final_awaiter<T> { std::coroutine_handle<task_promise<T>>::from_promise(*this) }; }

        template <typename ReturnType>
        _inline_always constexpr void return_value(ReturnType&& ret)
        {
            new(&this->value) T(std::forward<ReturnType>(ret));
            this->has_value = true;
        }

        friend struct sys::internal::task_awaiter<T>;
    private:
        union
        {
            byte _ = 0;
            T value;
        };
        bool has_value = false;
    };
    template <>
    struct task_promise<void> final : public task_promise_b<void>
    {
        std::coroutine_handle<> continuation = nullptr;
        std::exception_ptr exception = nullptr;

        _inline_always task_final_awaiter<void> final_suspend() noexcept { return task_final_awaiter<void> { std::coroutine_handle<task_promise<void>>::from_promise(*this) }; }

        _inline_always constexpr void return_void() const noexcept { }
    };
} // namespace sys::internal

namespace sys
{
    /// @brief Task coroutine.
    ///
    /// @tparam T Coroutine return type.
    /// @note Pass `byref`.
    template <typename T = void>
    class [[nodiscard]] task final
    {
    public:
        using promise_type = internal::task_promise<T>;

        /// @brief The longest possible delay in milliseconds, supported by `sys::task<>::delay(...)`.
        constexpr static i32 max_delay = ::sys::platform::task_max_delay;

        constexpr task() noexcept = default;
        constexpr task(const task&) = delete;
        constexpr task(task&& other) noexcept : handle(std::exchange(other.handle, nullptr)) { }
        _inline_always ~task()
        {
            if (this->handle)
                this->handle.destroy();
        }

        constexpr task& operator=(const task&) = delete;
        constexpr task& operator=(task&& other) noexcept
        {
            if (this != &other) [[likely]]
            {
                if (this->handle)
                    this->handle.destroy();
                this->handle = std::exchange(other.handle, nullptr);
            }
            return *this;
        }

        _inline_always internal::task_awaiter<T> operator co_await() { return internal::task_awaiter<T>(this->handle); }

        /// @brief Suspend and allow the implementation (i.e. a scheduler) to decide when to resume.
        static _impl_task_yield_rtype yield()
        requires (std::is_same_v<T, void>)
        {
            _impl_task_yield();
        }
        /// @brief Wait for at least `ms` milliseconds.
        static _impl_task_delay_rtype delay(i32 ms)
        requires (std::is_same_v<T, void>)
        {
            _impl_task_delay();
        }
        /// @brief Wait until `func()` (or `co_await func()`) is `true`.
        template <typename Pred>
        static task<void> wait_until(Pred func)
        requires (std::is_same_v<T, void>)
        {
            if constexpr (!std::convertible_to<decltype(func()), bool>)
                while (!co_await func())
                { }
            else
                while (!func())
                    co_await task<>::yield();
        }

        friend void swap(task& a, task& b) noexcept
        {
            using std::swap;
            swap(a.handle, b.handle);
        }

        friend struct sys::internal::task_promise_b<T>;
    private:
        std::coroutine_handle<internal::task_promise<T>> handle = nullptr;

        _inline_always explicit task(std::coroutine_handle<internal::task_promise<T>> handle) : handle(handle) { }
    };

    /// @brief Fire-and-forget launcher coroutine.
    struct async
    {
        using promise_type = internal::async_promise;

        friend struct internal::async_promise;
    private:
        _inline_always explicit async(std::coroutine_handle<internal::async_promise> handle) { ::sys::platform::launch_async(handle.address()); }
    };
} // namespace sys

sys::async sys::internal::async_promise::get_return_object() { return sys::async(std::coroutine_handle<sys::internal::async_promise>::from_promise(*this)); }

_pop_nowarn_msvc();

// NOLINTEND(misc-non-private-member-variables-in-classes)
