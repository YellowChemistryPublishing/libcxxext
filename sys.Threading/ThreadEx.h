#pragma once

/// @file

#define NOMINMAX 1 // NOLINT(readability-identifier-naming)
#include <concepts>
#include <cstddef>
#include <new>
#include <tinycthread.h>
#include <type_traits>
#include <utility>
#undef NOMINMAX // NOLINT(misc-include-cleaner): Spurious.
#ifdef call_once
#undef call_once
#endif

#include <CompilerWarnings.h>
#include <Destructor.h>
#include <Integer.h>
#include <LanguageSupport.h>
#include <Numeric.h>
#include <Result.h>
#include <ThreadingErrors.h>
#include <meta/NamedRequirements.h>

namespace sys
{
    class thread;

    /// @ingroup sys_threading
    /// @brief Opaque, non-owning identifier for a thread, or `nullptr`.
    /// @details
    /// Implements `sys::INothrowCopyConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowCopyAssignable`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`,
    /// `sys::IBooleanTestable`, `sys::INothrowEqualityComparable`, `sys::INothrowSwappable`.
    /// @see
    /// For more information on thread ids, see
    /// [Rust Docs](https://doc.rust-lang.org/std/thread/struct.ThreadId.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/thread/id.html).
    struct [[nodiscard]] thread_id final
    {
    private:
        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running, or empty.
        thread_id(const thrd_t th, decltype(unsafe)) noexcept : th(th) { }
    public:
        /// @brief Construct an empty thread id.
        /* NOLINT(hicpp-explicit-conversions) */ thread_id(std::nullptr_t) noexcept { }
        thread_id(const thread_id&) noexcept = default;
        thread_id(thread_id&& other) noexcept { swap(*this, other); }
        ~thread_id() noexcept = default;

        thread_id& operator=(const thread_id&) noexcept = default;
        thread_id& operator=(thread_id&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Whether this thread id is valid (i.e. non-empty).
        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        [[nodiscard]] friend bool operator==(const thread_id& a, const thread_id& b) noexcept { return thrd_equal(a.th, b.th); }

        friend void swap(thread_id& a, thread_id& b) noexcept { std::swap(a.th, b.th); }

        friend class sys::thread;
    };

    /// @ingroup sys_threading
    /// @brief Nullable-value specialization for `sys::result<thread_id, void>`.
    /// @details
    /// Implements `sys::INothrowCopyConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowCopyAssignable`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`,
    /// `sys::IBooleanTestable`, `sys::INothrowEqualityComparable`, `sys::INothrowSwappable`.
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

    /// @ingroup sys_threading
    /// @brief Encapsulates the information about a running thread, or `nullptr`.
    /// @details
    /// Implements `sys::INothrowMoveConstructible`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`, `sys::IBooleanTestable`, `sys::INothrowEqualityComparable`,
    /// `sys::INothrowSwappable`.
    /// @see
    /// For more information about thread handles, see
    /// [Rust Docs](https://doc.rust-lang.org/std/thread/struct.Thread.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/thread.html).
    class [[nodiscard]] thread final
    {
        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running, or empty.
        thread(const thrd_t th, decltype(unsafe)) noexcept : th(th) { }
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

        /// @brief Whether this thread is valid (i.e. non-empty).
        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        /// @brief Obtain the id of this thread.
        [[nodiscard]] thread_id id() const noexcept { return { this->th, unsafe }; }

        friend thread sys::thread_current() noexcept;
        friend void swap(thread& a, thread& b) noexcept { std::swap(a.th, b.th); }

        friend class sys::managed_thread;
    };

    /// @ingroup sys_threading
    /// @brief Obtains a handle to the current thread.
    [[nodiscard]] inline thread thread_current() noexcept { return { thrd_current(), unsafe }; }

    /// @ingroup sys_threading
    /// @brief Yields the current thread.
    inline void thread_yield() noexcept { thrd_yield(); }

    /// @ingroup sys_threading
    /// @brief A spun-off thread lifetime that is joined upon destruction.
    /// @details
    /// Implements `sys::INothrowMoveConstructible`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`, `sys::IBooleanTestable`, `sys::INothrowSwappable`.
    /// @see
    /// For more information about joining thread handles, see
    /// [Rust Docs](https://doc.rust-lang.org/std/thread/struct.JoinHandle.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/jthread.html).
    class [[nodiscard]] managed_thread final
    {
        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running.
        managed_thread(const thrd_t th, decltype(unsafe)) noexcept : th(th) { }
    public:
        /// @brief Construct an empty managed thread.
        /* NOLINT(hicpp-explicit-conversions) */ managed_thread(std::nullptr_t) noexcept { }
        managed_thread(const managed_thread&) noexcept = delete;
        managed_thread(managed_thread&& other) noexcept { swap(*this, other); }
        ~managed_thread() noexcept /* NOLINT(bugprone-exception-escape) */
        {
            _nowarn_begin_one_gcc("-Wterminate");
            _nowarn_begin_one_clang(_clwarn_clang_exceptions);
            _nowarn_begin_one_msvc(_clwarn_msvc_function_function_assumed_not_to_throw_an_exception_but_does);
            if (this->joinable())
                _contract_assert(thrd_join(this->th, nullptr) == thrd_success, "If this happens we're genuinely cooked.");
            _nowarn_end_msvc();
            _nowarn_end_clang();
            _nowarn_end_gcc();
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
        requires IFunctionObject<std::decay_t<Func>> && INothrowDestructible<std::decay_t<Func>>
        static result<managed_thread, threading_error> ctor(Func&& func) noexcept(noexcept(std::decay_t<Func>(std::declval<Func&&>())))
        {
            thrd_t th {};

            std::decay_t<Func>* f = [&]() noexcept(noexcept(std::decay_t<Func>(std::declval<Func&&>()))) -> std::decay_t<Func>*
            {
                return new(std::nothrow) std::decay_t<Func>(std::forward<Func>(func)); // NOLINT(cppcoreguidelines-owning-memory)
            }();
            _retif(threading_error::oom, !f);

            sys::optional_destructor releaseFunc = [f]() noexcept -> void { delete f /* NOLINT(cppcoreguidelines-owning-memory) */; };

            _nowarn_begin_one_msvc(4702);
            if (thrd_create(&th, [](void* arg) noexcept -> int
            {
                int ret = 0;
                std::decay_t<Func>* func = _as(arg, std::decay_t<Func>*);
                sys::destructor releaseFunc = [func]() noexcept -> void { delete func /* NOLINT(cppcoreguidelines-owning-memory) */; };

                try
                {
                    if constexpr (std::convertible_to<std::invoke_result_t<std::decay_t<Func>>, int>)
                        ret = _as((*func)(), int);
                    else
                        (void)(*func)();
                }
                catch (...)
                {
                    return sys::bsentinel<int>();
                }

                return ret;
            }, _as(f, void*)) != thrd_success)
                return threading_error::init_failed;
            _nowarn_end_msvc();

            releaseFunc.clear();
            return managed_thread(th, unsafe);
        }

        /// @brief Whether this managed thread is valid (i.e. non-empty).
        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        /// @brief Whether this managed thread is joinable.
        [[nodiscard]] bool joinable() const noexcept { return this->th != thrd_t {} && !thrd_equal(this->th, thrd_current()); }

        /// @brief Obtain information about this thread.
        [[nodiscard]] sys::thread thread() const noexcept { return { this->th, unsafe }; }

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

        friend void swap(managed_thread& a, managed_thread& b) noexcept { std::swap(a.th, b.th); }
    };
} // namespace sys

#undef timespec // NOLINT(misc-include-cleaner)
