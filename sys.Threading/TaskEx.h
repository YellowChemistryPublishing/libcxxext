#pragma once

#include <CompilerWarnings.h>
_push_nowarn_msvc(_clWarn_msvc_unreachable) // Erroneously generated for compiler coroutine codegen.

#include <coroutine>

#include <LanguageSupport.h>
#include <rt.h>
#include <rt_threading.h>

    namespace sys
{
    template <typename T>
    struct task_promise;

    template <typename T>
    class task;

    template <typename T>
    struct task_awaiter
    {
        std::coroutine_handle<task_promise<T>> handle;

        _inline_always constexpr bool await_ready() const noexcept
        {
            return !this->handle || this->handle.done();
        }
        _inline_never auto await_suspend(std::coroutine_handle<> parent)
        {
            this->handle.promise().continuation = parent;
            _task_yield_and_resume();
        }
        _inline_always constexpr T await_resume() const noexcept(std::is_same<T, void>::value)
        {
            if (this->handle.promise().exception) [[unlikely]]
                std::rethrow_exception(this->handle.promise().exception);
            if constexpr (!std::is_same<T, void>::value)
                return std::move(*reinterpret_cast<T*>(&this->handle.promise().value));
        }
    };
    template <typename T>
    struct task_final_awaiter
    {
        std::coroutine_handle<task_promise<T>> handle;

        _inline_always constexpr bool await_ready() const noexcept
        {
            return false;
        }
        _inline_never auto await_suspend(std::coroutine_handle<>) noexcept
        {
            _task_yield_and_continue();
        }
        constexpr void await_resume() const noexcept
        { }
    };

    template <typename T>
    struct task_promise_b
    {
        std::coroutine_handle<> continuation;
        std::exception_ptr exception = nullptr;

        inline static void* operator new(size_t sz) noexcept
        {
            return ::sys::platform::_task_operator_new(sz);
        }
        inline static void operator delete(void* ptr) noexcept
        {
            ::sys::platform::_task_operator_delete(ptr);
        }

        _inline_always constexpr std::suspend_always initial_suspend() const noexcept
        {
            return std::suspend_always();
        }

        _inline_always task<T> get_return_object()
        {
            return task<T>(std::coroutine_handle<task_promise<T>>::from_promise(*static_cast<task_promise<T>*>(this)));
        }

        [[noreturn]] inline static task<T> get_return_object_on_allocation_failure()
        {
            _throw(std::bad_alloc());
        }
        _inline_always void unhandled_exception()
        {
            this->exception = std::current_exception();
        }
    };
    template <typename T>
    struct task_promise final : public task_promise_b<T>
    {
        union
        {
            byte _;
            T value;
        };
        bool hasValue = false;

        constexpr task_promise() noexcept
        { }
        inline ~task_promise() noexcept(noexcept(this->value.~T()))
        {
            if (this->hasValue) [[likely]]
                this->value.~T();
        }

        _inline_always task_final_awaiter<T> final_suspend() noexcept
        {
            return task_final_awaiter<T> { std::coroutine_handle<task_promise<T>>::from_promise(*this) };
        }

        template <typename ReturnType>
        _inline_always constexpr void return_value(ReturnType&& ret)
        {
            new(&this->value) T(std::forward<ReturnType>(ret));
            this->hasValue = true;
        }
    };
    template <>
    struct task_promise<void> final : public task_promise_b<void>
    {
        _inline_always task_final_awaiter<void> final_suspend() noexcept
        {
            return task_final_awaiter<void> { std::coroutine_handle<task_promise<void>>::from_promise(*this) };
        }

        _inline_always constexpr void return_void() const noexcept
        { }
    };

    template <typename T = void>
    class [[nodiscard]] task final
    {
    public:
        using promise_type = task_promise<T>;

        constexpr static i32 max_delay = ::sys::platform::_task_max_delay;

        _inline_always ~task()
        {
            if (this->handle)
                this->handle.destroy();
        }

        _inline_always task_awaiter<T> operator co_await()
        {
            return task_awaiter<T>(this->handle);
        }

        // clang-format off: Looks better stacked vertically, we can't put a semicolon after these!
        _impl_task_yield()
        _impl_task_delay()
        template <typename Pred>
        inline static task<void> wait_until(Pred&& func)
        // clang-format on
        {
            if constexpr (!std::convertible_to<decltype(func()), bool>)
                while (!co_await func());
            else
                while (!func()) co_await task<>::yield();
        }

        friend struct sys::task_promise_b<T>;
    private:
        std::coroutine_handle<task_promise<T>> handle;

        _inline_always explicit task(std::coroutine_handle<task_promise<T>> handle) : handle(handle)
        { }
    };

    struct async;

    struct async_promise
    {
        constexpr async_promise() noexcept = default;
        _inline_always constexpr ~async_promise() = default;

        _inline_always async get_return_object();

        _inline_always constexpr std::suspend_always initial_suspend() const noexcept
        {
            return std::suspend_always();
        }
        _inline_always constexpr std::suspend_never final_suspend() const noexcept
        {
            return std::suspend_never();
        }

        inline void unhandled_exception() noexcept
        {
            std::rethrow_exception(std::current_exception());
        }
        _inline_always constexpr void return_void() const noexcept
        { }
    };

    struct async
    {
        using promise_type = async_promise;

        friend struct async_promise;
    private:
        _inline_always explicit async(std::coroutine_handle<async_promise> handle)
        {
            ::sys::platform::_launch_async(handle.address());
        }
    };

    async async_promise::get_return_object()
    {
        return async(std::coroutine_handle<async_promise>::from_promise(*this));
    }
} // namespace sys

_pop_nowarn_msvc();
