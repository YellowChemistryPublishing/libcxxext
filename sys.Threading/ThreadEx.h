#pragma once

/// @file

#include <concepts>
#include <new>
#include <tinycthread.h>
#include <type_traits>
#include <utility>

#include <Destructor.h>
#include <LanguageSupport.h>
#include <Numeric.h>
#include <Result.h>
#include <ThreadingErrors.h>

namespace sys
{
    class thread_handle final
    {
        /// @brief Checks if `Func` could be the type of a global function.
        template <typename Func>
        static consteval bool is_global_func() noexcept
        {
            return std::is_function_v<std::remove_pointer_t<std::decay_t<Func>>>;
        }
        template <typename Func>
        using storage_type = std::conditional_t<thread_handle::is_global_func<Func>(), std::decay_t<Func>, std::decay_t<Func>*>;

        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running.
        thread_handle(const thrd_t th, unsafe) noexcept : th(th) { }
    public:
        /// @brief Empty, non-joinable.
        thread_handle() noexcept = default;
        thread_handle(const thread_handle&) = delete;
        thread_handle(thread_handle&& other) noexcept { std::swap(this->th, other.th); }
        ~thread_handle() noexcept
        {
            if (this->joinable())
                _contract_assert(thrd_join(this->th, nullptr) == thrd_success, "If this happens we're genuinely cooked.");
        }

        thread_handle& operator=(const thread_handle&) = delete;
        thread_handle& operator=(thread_handle&& other) noexcept
        {
            std::swap(this->th, other.th);
            return *this;
        }

        /// @brief Create a `sys::thread_handle` with `func`.
        template <typename Func>
        static result<thread_handle, threading_error> ctor(Func&& func) noexcept(noexcept(auto(std::forward<Func>(func)())))
        requires requires {
            { func() };
        };

        /// @brief Whether this thread is joinable.
        [[nodiscard]] explicit operator bool() const noexcept { return this->joinable(); }
        [[nodiscard]] bool joinable() const noexcept { return this->th != thrd_t {}; }

        result<sys::integer<int>, threading_error> join() noexcept
        {
            _retif(threading_error::invalid_operation, !this->joinable());

            sys::integer<int> res = 0;
            if (thrd_join(this->th, &*res) != thrd_success)
                return threading_error::operation_failed;
            this->th = thrd_t {};
            return res;
        }
        void detach() noexcept
        {
            _retif(, !this->joinable());

            (void)thrd_detach(this->th);
            this->th = thrd_t {};
        }

        friend void swap(thread_handle& a, thread_handle& b) noexcept { std::swap(a.th, b.th); }
    };

    template <typename Func>
    inline result<thread_handle, threading_error> thread_handle::ctor(Func&& func) noexcept(noexcept(auto(std::forward<Func>(func)())))
    requires requires {
        { func() };
    }
    {
        thrd_t th {};

        storage_type<Func> f = [&] noexcept
        {
            if constexpr (!thread_handle::is_global_func<Func>())
                return new(std::nothrow) std::decay_t<Func>(std::forward<Func>(func)); // NOLINT(cppcoreguidelines-owning-memory)
            else
                return func;
        }();
        if constexpr (thread_handle::is_global_func<Func>())
            _retif(threading_error::invalid_argument, !f);
        else
            _retif(threading_error::oom, !f);

        sys::optional_destructor releaseFunc = [&] noexcept
        {
            if constexpr (!thread_handle::is_global_func<Func>())
                delete f; // NOLINT(cppcoreguidelines-owning-memory)
        };

        if (thrd_create(&th, [](void* arg) noexcept(noexcept(func())) -> int
        {
            int ret = 0;
            std::decay_t<Func> func = [&]()
            {
                if constexpr (!thread_handle::is_global_func<Func>())
                    return *(_as(std::decay_t<Func>*, arg));
                else
                    return _asr(std::decay_t<Func>, arg);
            }();

            try
            {
                if constexpr (requires {
                                  { func() } -> std::convertible_to<int>;
                              })
                    ret = _as(int, func());
                else
                    (void)func();
            }
            catch (...)
            {
                return sys::bsentinel<int>();
            }

            return ret;
        }, _asr(void*, f)) != thrd_success)
            return threading_error::init_failed;

        releaseFunc.release(unsafe());
        return thread_handle(th, unsafe());
    }

    /// @brief Yields the current thread.
    inline void thread_yield() noexcept { thrd_yield(); }

    /// @brief Exits the current thread with the given return code.
    inline void thread_exit(const int ret) noexcept { thrd_exit(ret); }
} // namespace sys

#undef timespec // NOLINT(misc-include-cleaner)
