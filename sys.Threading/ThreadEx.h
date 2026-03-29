#pragma once

/// @file

#define NOMINMAX 1 // NOLINT(readability-identifier-naming)
#include <concepts>
#include <cstddef>
#include <new>
#include <tinycthread.h>
#include <type_traits>
#include <utility>
#undef NOMINMAX
#ifdef call_once
#undef call_once
#endif

#include <Destructor.h>
#include <LanguageSupport.h>
#include <Numeric.h>
#include <Result.h>
#include <ThreadingErrors.h>

namespace sys
{
    class thread;

    /// @brief Opaque, non-owning identifier for a thread, or `nullptr`.
    /// @details
    /// Implements `std::is_nothrow_copy_constructible_v`, `std::is_nothrow_move_constructible_v`, `std::is_nothrow_copy_assignable_v`, `std::is_nothrow_move_assignable_v`,
    /// `std::is_nothrow_destructible_v`, `std::equality_comparable`.
    /// @see
    /// For more information on thread ids, see
    /// [Rust Docs](https://doc.rust-lang.org/std/thread/struct.ThreadId.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/thread/id.html).
    struct thread_id
    {
    private:
        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running, or empty.
        thread_id(const thrd_t th, unsafe) noexcept : th(th) { }
    public:
        /// @brief Construct an empty thread id.
        /* NOLINT(hicpp-explicit-conversions) */ thread_id(std::nullptr_t) noexcept { }
        thread_id(const thread_id&) noexcept = default;
        thread_id(thread_id&&) noexcept = default;
        ~thread_id() noexcept = default;

        thread_id& operator=(const thread_id&) noexcept = default;
        thread_id& operator=(thread_id&&) noexcept = default;

        /// @brief Whether this thread id is valid.
        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        [[nodiscard]] friend bool operator==(const thread_id& a, const thread_id& b) noexcept { return thrd_equal(a.th, b.th); }

        friend class sys::thread;
    };

    /// @brief Nullable-value specialization for `sys::result<thread_id, void>`.
    template <>
    class [[nodiscard]] result<thread_id, void> final : public internal::nullable_value_result<thread_id>
    {
    public:
        using internal::nullable_value_result<thread_id>::nullable_value_result;
        using internal::nullable_value_result<thread_id>::operator=;

        /// @brief Construct an error result.
        /* NOLINT(hicpp-explicit-conversions) */ result(std::nullptr_t) noexcept : internal::nullable_value_result<thread_id>(nullptr) { }
    };

    class managed_thread;
    thread thread_current() noexcept;

    /// @brief Encapsulates the information about a running thread, or `nullptr`.
    /// @details
    /// Implements `std::is_nothrow_copy_constructible_v`, `std::is_nothrow_move_constructible_v`, `std::is_nothrow_copy_assignable_v`, `std::is_nothrow_move_assignable_v`,
    /// `std::is_nothrow_destructible_v`, `std::boolean_testable`.
    /// @see
    /// For more information about thread handles, see
    /// [Rust Docs](https://doc.rust-lang.org/std/thread/struct.Thread.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/thread.html).
    class thread final
    {
        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running, or empty.
        thread(const thrd_t th, unsafe) noexcept : th(th) { }
    public:
        /// @brief Construct an empty thread handle.
        /* NOLINT(hicpp-explicit-conversions) */ thread(std::nullptr_t) noexcept { }
        thread(const thread&) noexcept = delete;
        thread(thread&& other) noexcept { swap(*this, other); }
        ~thread() noexcept = default;

        thread& operator=(const thread&) noexcept = delete;
        thread& operator=(thread&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Whether this thread is valid.
        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        /// @brief Obtain the id of this thread.
        [[nodiscard]] thread_id id() const noexcept { return { this->th, unsafe() }; }

        /// @brief Swap two `sys::thread`.
        friend void swap(thread& a, thread& b) noexcept { std::swap(a.th, b.th); }
        friend thread sys::thread_current() noexcept;

        friend class sys::managed_thread;
    };

    /// @brief Obtains a handle to the current thread.
    inline thread thread_current() noexcept { return { thrd_current(), unsafe() }; }

    /// @brief Yields the current thread.
    inline void thread_yield() noexcept { thrd_yield(); }

    /// @brief Exits the current thread with the given return code.
    inline void thread_exit(const int ret) noexcept { thrd_exit(ret); }

    /// @brief A spun-off thread lifetime that is joined upon destruction.
    /// @details
    /// Implements `std::is_nothrow_move_constructible_v`, `std::is_nothrow_move_assignable_v`, `std::is_nothrow_destructible_v`, `std::boolean_testable`.
    /// @see
    /// For more information about joining thread handles, see
    /// [Rust Docs](https://doc.rust-lang.org/std/thread/struct.JoinHandle.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/jthread.html).
    class managed_thread final
    {
        /// @brief Checks if `Func` could be the type of a global function.
        template <typename Func>
        static consteval bool is_global_func() noexcept
        {
            return std::is_function_v<std::remove_pointer_t<std::decay_t<Func>>>;
        }
        template <typename Func>
        using storage_type = std::conditional_t<managed_thread::is_global_func<Func>(), std::decay_t<Func>, std::decay_t<Func>*>;

        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running.
        managed_thread(const thrd_t th, unsafe) noexcept : th(th) { }
    public:
        /// @brief Construct an empty managed thread.
        /* NOLINT(hicpp-explicit-conversions) */ managed_thread(std::nullptr_t) noexcept { }
        managed_thread(const managed_thread&) noexcept = delete;
        managed_thread(managed_thread&& other) noexcept { swap(*this, other); }
        ~managed_thread() noexcept
        {
            if (this->joinable())
                _contract_assert(thrd_join(this->th, nullptr) == thrd_success, "If this happens we're genuinely cooked.");
        }

        managed_thread& operator=(const managed_thread&) noexcept = delete;
        managed_thread& operator=(managed_thread&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Trampoline off a `sys::managed_thread` executing `func`.
        /// @details
        /// If `func()` returns exactly `int`, its successful invocation produces `.join() == func()`.
        /// If the thread executing `func()` exits via `sys::thread_exit(x)`, then `.join() == x`.
        /// If `func()` throws, the result of `.join()` is `sys::bsentinel<int>()`.
        /// Otherwise, `.join()` returns `0`.
        template <typename Func>
        static result<managed_thread, threading_error> ctor(Func&& func) noexcept(noexcept(auto(std::forward<Func>(func)())))
        requires requires {
            { func() };
        }
        {
            thrd_t th {};

            storage_type<Func> f = [&]() noexcept -> storage_type<Func>
            {
                if constexpr (!managed_thread::is_global_func<Func>())
                    return new(std::nothrow) std::decay_t<Func>(std::forward<Func>(func)); // NOLINT(cppcoreguidelines-owning-memory)
                else
                    return func;
            }();
            if constexpr (managed_thread::is_global_func<Func>())
                _retif(threading_error::invalid_argument, !f);
            else
                _retif(threading_error::oom, !f);

            sys::optional_destructor releaseFunc = [&]() noexcept -> void
            {
                if constexpr (!managed_thread::is_global_func<Func>())
                    delete f; // NOLINT(cppcoreguidelines-owning-memory)
            };

            if (thrd_create(&th, [](void* arg) noexcept -> int
            {
                int ret = 0;
                std::decay_t<Func> func = [&]() -> std::decay_t<Func>
                {
                    if constexpr (!managed_thread::is_global_func<Func>())
                        return *(_as(std::decay_t<Func>*, arg));
                    else
                        return _asr(std::decay_t<Func>, arg);
                }();

                try
                {
                    if constexpr (std::convertible_to<std::invoke_result_t<std::decay_t<Func>>, int>)
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
            return managed_thread(th, unsafe());
        }

        /// @brief Whether this managed thread is valid.
        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        /// @brief Whether this managed thread is joinable.
        [[nodiscard]] bool joinable() const noexcept { return this->th != thrd_t {} && !thrd_equal(this->th, thrd_current()); }

        /// @brief Obtain information about this thread.
        [[nodiscard]] thread thread() const noexcept { return { this->th, unsafe() }; }

        /// @brief Block-and-wait for this thread to complete execution, and obtain its exit code.
        result<sys::integer<int>, threading_error> join() noexcept
        {
            _retif(threading_error::invalid_operation, !this->joinable());

            sys::integer<int> res = 0;
            if (thrd_join(this->th, &*res) != thrd_success)
                return threading_error::operation_failed;
            this->th = thrd_t {};
            return res;
        }
        /// @brief Release ownership of the thread.
        void detach() noexcept
        {
            _retif(, !this->joinable());

            (void)thrd_detach(this->th);
            this->th = thrd_t {};
        }

        /// @brief Swap two `sys::managed_thread`.
        friend void swap(managed_thread& a, managed_thread& b) noexcept { std::swap(a.th, b.th); }
    };
} // namespace sys

#undef timespec // NOLINT(misc-include-cleaner)
