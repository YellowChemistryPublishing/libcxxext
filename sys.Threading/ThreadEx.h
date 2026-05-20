#pragma once

/// @file

#include <concepts>
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

#include <CompilerWarnings.h>
#include <Destructor.h>
#include <Integer.h>
#include <LanguageSupport.h>
#include <Numeric.h>
#include <Result.h>
#include <ThreadingErrors.h>
#include <meta/InterfaceRequirements.h>
#include <meta/NamedRequirements.h>
#include <meta/Type.h>

#if !defined(_libcxxext_internal_mock_sup_thread) || !_libcxxext_internal_mock_sup_thread
#include <sup/ThreadHandle.h>
#include <sup/ThreadingErrors.h>
#endif

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
        internal::thread_handle th = nullptr;

        /// @warning `unsafe` because `th` must be initialized and running, or empty.
        thread_id(const internal::thread_handle th, decltype(unsafe)) noexcept : th(th) { }
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
        [[nodiscard]] explicit operator bool() const noexcept { return _as(this->th, bool); }
        [[nodiscard]] friend bool operator==(const thread_id& a, const thread_id& b) noexcept { return a.th == b.th; }

        friend void swap(thread_id& a, thread_id& b) noexcept { std::swap(a.th, b.th); }

        friend class sys::thread;
    };

    /// @ingroup sys_threading
    /// @brief Nullable-value specialization for `sys::result<sys::thread_id, void>`.
    /// @details
    /// Implements `sys::INothrowCopyConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowCopyAssignable`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`,
    /// `sys::IBooleanTestable`, `sys::INothrowEqualityComparable`, `sys::INothrowSwappable`.
    template <>
    class [[nodiscard]] result<thread_id, void> final : public internal::nullable_value_result<thread_id>
    {
    public:
        using internal::nullable_value_result<thread_id>::nullable_value_result;
        using internal::nullable_value_result<thread_id>::operator=;
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
        internal::thread_handle th = nullptr;

        /// @warning `unsafe` because `th` must be initialized and running, or empty.
        thread(const internal::thread_handle th, decltype(unsafe)) noexcept : th(th) { }
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
        [[nodiscard]] explicit operator bool() const noexcept { return _as(this->th, bool); }
        /// @brief Obtain the id of this thread.
        [[nodiscard]] thread_id id() const noexcept { return { this->th, unsafe }; }

        friend thread sys::thread_current() noexcept;
        friend void swap(thread& a, thread& b) noexcept { std::swap(a.th, b.th); }

        friend class sys::managed_thread;
    };

    /// @ingroup sys_threading
    /// @brief Nullable-value specialization for `sys::result<sys::thread, void>`.
    /// @details
    /// Implements `sys::INothrowCopyConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowCopyAssignable`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`,
    /// `sys::IBooleanTestable`, `sys::INothrowEqualityComparable`, `sys::INothrowSwappable`.
    template <>
    class [[nodiscard]] result<sys::thread, void> final : public internal::nullable_value_result<sys::thread>
    {
    public:
        using internal::nullable_value_result<sys::thread>::nullable_value_result;
        using internal::nullable_value_result<sys::thread>::operator=;
    };

    /// @ingroup sys_threading
    /// @brief Obtains a handle to the current thread.
    [[nodiscard]] inline thread thread_current() noexcept { return { internal::thread_current(unsafe), unsafe }; }

    /// @ingroup sys_threading
    /// @brief Yields the current thread.
    inline void thread_yield() noexcept { internal::thread_yield(unsafe); }

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
        internal::thread_handle th = nullptr;

        /// @warning `unsafe` because `th` must be initialized and running.
        managed_thread(const internal::thread_handle th, decltype(unsafe)) noexcept : th(th) { }
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
                _contract_assert(this->th.join(nullptr, unsafe) == internal::threading_error::ok, "If this happens we're genuinely cooked."); // LCOV_EXCL_BR_LINE
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
        /// If `func()` throws, the result of `.join()` is `sys::bsentinel<int>()`.
        /// Otherwise, `.join()` returns `0`.
        template <typename Func>
        requires IFunctionObject<std::decay_t<Func>> && INothrowDestructible<std::decay_t<Func>>
        static auto ctor(Func&& func) noexcept(INothrowConstructibleFrom<std::decay_t<Func>, decltype(_forward(func))>)
        {
            internal::thread_handle th = nullptr;

            std::decay_t<Func>* f = [&]() noexcept(INothrowConstructibleFrom<std::decay_t<Func>, decltype(_forward(func))>) -> std::decay_t<Func>*
            { return new(std::nothrow) std::decay_t<Func>(_forward(func)) /* NOLINT(cppcoreguidelines-owning-memory)*/; }(); // LCOV_EXCL_BR_LINE
            _retif((result<managed_thread, threading_error>(threading_error::oom)), !f);

            sys::optional_destructor onFail = [f]() noexcept -> void { ::operator delete(f /* NOLINT(cppcoreguidelines-owning-memory) */, std::nothrow); }; // LCOV_EXCL_BR_LINE

            _nowarn_begin_one_msvc(4702);
            if (th.create([](void* arg) noexcept -> int
            {
                int ret = 0; // NOLINT(misc-const-correctness): Not on the other `constexpr` branch!
                const std::decay_t<Func>* func = _as(arg, std::decay_t<Func>*);
                _defer([func]() noexcept -> void { delete func /* NOLINT(cppcoreguidelines-owning-memory) */; }); // LCOV_EXCL_BR_LINE

                try
                {
                    if constexpr (std::convertible_to<std::invoke_result_t<std::decay_t<Func>>, int>)
                        ret = _as((*func)(), int); // LCOV_EXCL_BR_LINE
                    else
                        (void)(*func)();
                }
                catch (...)
                {
                    return sys::bsentinel<int>();
                }

                return ret;
            }, _as(f, void*), unsafe) != internal::threading_error::ok)
                return result<managed_thread, threading_error>(threading_error::init_failed);
            _nowarn_end_msvc();

            onFail.clear();
            return result<managed_thread, threading_error>(managed_thread(th, unsafe));
        }

        /// @brief Whether this managed thread is valid (i.e. non-empty).
        [[nodiscard]] explicit operator bool() const noexcept { return _as(this->th, bool); }
        /// @brief Whether this managed thread is joinable.
        [[nodiscard]] bool joinable() const noexcept { return this->th && this->th != internal::thread_current(unsafe); } // LCOV_EXCL_BR_LINE

        /// @brief Obtain information about this thread.
        [[nodiscard]] sys::thread thread() const noexcept { return { this->th, unsafe }; }

        /// @brief Block-and-wait for this thread to complete execution, and obtain its exit code.
        result<sys::integer<int>, threading_error> join() noexcept
        {
            _retif(threading_error::invalid_operation, !this->joinable());

            sys::integer<int> res(0);
            if (this->th.join(&*res, unsafe) != internal::threading_error::ok)
                return threading_error::operation_failed;
            this->th = nullptr;
            return res;
        }
        /// @brief Release ownership of the thread.
        void detach() noexcept
        {
            _retif(, !this->joinable());

            (void)this->th.detach(unsafe);
            this->th = nullptr;
        }

        friend void swap(managed_thread& a, managed_thread& b) noexcept { std::swap(a.th, b.th); }
    };

    /// @ingroup sys_threading
    /// @brief Nullable-value specialization for `sys::result<sys::managed_thread, void>`.
    /// @details
    /// Implements `sys::INothrowCopyConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowCopyAssignable`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`,
    /// `sys::IBooleanTestable`, `sys::INothrowEqualityComparable`, `sys::INothrowSwappable`.
    template <>
    class [[nodiscard]] result<sys::managed_thread, void> final : public internal::nullable_value_result<sys::managed_thread>
    {
    public:
        using internal::nullable_value_result<sys::managed_thread>::nullable_value_result;
        using internal::nullable_value_result<sys::managed_thread>::operator=;
    };
} // namespace sys

#undef timespec // NOLINT(misc-include-cleaner)
