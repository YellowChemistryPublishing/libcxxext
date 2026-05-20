#pragma once

/// @file

#include <CompilerWarnings.h>
#include <LanguageSupport.h>
#include <Once.h>
#include <ResourceGuard.h>
#include <Result.h>
#include <ThreadingErrors.h>

#if !defined(_libcxxext_internal_mock_sup_mut) || !_libcxxext_internal_mock_sup_mut
#include <sup/MutexHandle.h>
#include <sup/ThreadingErrors.h>
#endif

namespace sys
{
    class cond_var;

    /// @ingroup sys_threading
    /// @brief An optionally reentrant mutex threading primitive.
    /// @details Implements `sys::INothrowDefaultConstructible` and `sys::INothrowDestructible`.
    /// @attention Unless you're writing generic code, you probably want either `sys::mutex` or `sys::reentrant_mutex`.
    template <bool IsRecursive>
    class [[clang::capability("mutex")]] ordinary_mutex final
    {
        internal::mutex_handle mut = nullptr;
        once o;

        result<void> try_init() noexcept
        {
            return this->o.call_once([&]() noexcept -> sys::result<void>
            {
                _retif(nullptr, this->mut.create<IsRecursive>() != internal::threading_error::ok);
                return {};
            });
        }

        /// @brief Run a possibly-failing, acquire routine.
        /// @pre `this->try_init() && this->o.is_completed()`
        /// @warning `unsafe` because `this` has preconditions.
        template <internal::threading_error (internal::mutex_handle::*Acquire)()>
        bool acq(internal::threading_error& ret, decltype(unsafe)) noexcept
        {
            return (ret = (this->mut.*Acquire)()) == internal::threading_error::ok;
        }
    public:
        ordinary_mutex() noexcept = default;
        ordinary_mutex(const ordinary_mutex&) noexcept = delete;
        ordinary_mutex(ordinary_mutex&&) noexcept = delete;
        ~ordinary_mutex() noexcept
        {
            if (this->o.is_completed()) [[likely]]
                this->mut.destroy();
        }

        ordinary_mutex& operator=(const ordinary_mutex&) noexcept = delete;
        ordinary_mutex& operator=(ordinary_mutex&&) noexcept = delete;

        /// @brief Acquire the lock on `this`, blocking until successful, or an error occurs.
        /// @pre If this mutex is not reentrant, `this` must not already be locked, otherwise, every prior reentrant acquisition must be by the current calling thread.
        /// @attention Lifetime assumptions!
        /// @code{.cpp}
        /// sys::managed_thread t = sys::managed_thread::ctor([]() /* noexcept -> ... */
        /// {
        ///     sys::ordinary_mutex<...> m /* = ... */;
        ///     ...
        ///     m.(try_)acquire(sys::unsafe); // If reentrant, not called prior or called at least once in the current thread, otherwise, not called prior.
        /// }).move();
        /// @endcode
        /// @warning `unsafe` because `this` has preconditions.
        result<void, threading_error> acquire(decltype(unsafe)) noexcept
        {
            internal::threading_error ret = internal::threading_error::ok;
            _retif(threading_error::init_failed, !this->try_init());
            _retif(threading_error::operation_failed, !this->acq<&internal::mutex_handle::lock>(ret, unsafe));
            return {};
        }
        /// @brief Try to acquire the lock on `this`, returning `threading_error::operation_failed` immediately if the lock is already held.
        /// @pre If this mutex is not reentrant, `this` must not already be locked, otherwise, every prior reentrant acquisition must be by the current calling thread.
        /// @attention Lifetime assumptions!
        /// @code{.cpp}
        /// sys::managed_thread t = sys::managed_thread::ctor([]() /* noexcept -> ... */
        /// {
        ///     sys::ordinary_mutex<...> m /* = ... */;
        ///     ...
        ///     m.(try_)acquire(sys::unsafe); // If reentrant, not called prior or called at least once in the current thread, otherwise, not called prior.
        /// }).move();
        /// @endcode
        /// @warning `unsafe` because `this` has preconditions.
        result<void, threading_error> try_acquire(decltype(unsafe)) noexcept
        {
            internal::threading_error ret = internal::threading_error::ok;
            _retif(threading_error::init_failed, !this->try_init());
            _retif(threading_error::busy, !this->acq<&internal::mutex_handle::try_lock>(ret, unsafe) && ret == internal::threading_error::busy);
            _retif(threading_error::operation_failed, ret != internal::threading_error::ok);
            return {};
        }
        /// @brief Release the lock on `this`.
        /// @pre `this` must be locked by the current thread.
        /// @attention Lifetime assumptions!
        /// @code{.cpp}
        /// sys::managed_thread t = sys::managed_thread::ctor([]() /* noexcept -> ... */
        /// {
        ///     sys::ordinary_mutex<...> m /* = ... */;
        ///     ...
        ///     m.acquire(sys::unsafe); // If not reentrant, called exactly once prior, otherwise, at least once, in the current thread.
        ///     ...
        ///     // The number of calls to release must be less than or equal to the number of successful calls to acquire.
        ///     // For a non-reentrant mutex, that will be exactly once.
        ///     // If the count is less, no other thread may acquire the lock.
        ///     m.release(sys::unsafe);
        /// }).move();
        /// @endcode
        /// @warning `unsafe` because `this` has preconditions.
        result<void, threading_error> release(decltype(unsafe)) noexcept /* NOLINT(bugprone-exception-escape) */
        {
            _nowarn_begin_one_gcc("-Wterminate");
            _nowarn_begin_one_clang(_clwarn_clang_exceptions);
            _nowarn_begin_one_msvc(_clwarn_msvc_function_function_assumed_not_to_throw_an_exception_but_does);
            _contract_assert(this->o.is_completed(), "`.acquire()` never called!"); // LCOV_EXCL_BR_LINE
            _nowarn_end_msvc();
            _nowarn_end_clang();
            _nowarn_end_gcc();

            _retif(threading_error::operation_failed, this->mut.unlock() != internal::threading_error::ok);
            return {};
        }
    private:
        static void release_guard(ordinary_mutex& m) noexcept /* NOLINT(bugprone-exception-escape) */
        {
            _nowarn_begin_one_gcc("-Wterminate");
            _nowarn_begin_one_clang(_clwarn_clang_exceptions);
            _nowarn_begin_one_msvc(_clwarn_msvc_function_function_assumed_not_to_throw_an_exception_but_does);
            _contract_assert(m.release(unsafe), "If this happens we're genuinely cooked."); // LCOV_EXCL_BR_LINE
            _nowarn_end_msvc();
            _nowarn_end_clang();
            _nowarn_end_gcc();
        };
    public:
        /// @brief RAII guard for `sys::ordinary_mutex`.
        /// @details
        /// Implements `sys::INothrowDefaultConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`, `sys::INothrowSwappable`.
        using guard = resource_guard<ordinary_mutex, &ordinary_mutex::release_guard>;

        /// @brief Lock and obtain a lock guard for this mutex.
        result<guard, threading_error> lock() noexcept
        {
            _retif(threading_error::init_failed, !this->try_init());

            internal::threading_error ret = internal::threading_error::ok;
            _retif(threading_error::operation_failed, !this->acq<&internal::mutex_handle::lock>(ret, unsafe));
            return guard(*this, unsafe);
        }
        /// @brief Try to lock this mutex and obtain a lock guard for it, if possible.
        result<guard, threading_error> try_lock() noexcept
        {
            _retif(threading_error::init_failed, !this->try_init());

            internal::threading_error ret = internal::threading_error::ok;
            _retif(threading_error::busy, !this->acq<&internal::mutex_handle::try_lock>(ret, unsafe) && ret == internal::threading_error::busy);
            _retif(threading_error::operation_failed, ret != internal::threading_error::ok);
            return guard(*this, unsafe);
        }
        // TODO(halloimdragon): `try_lock_for(...)`.

        friend class sys::cond_var;
    };

    /// @ingroup sys_threading
    /// @relates ordinary_mutex
    /// @brief The academic, ordinary mutex threading primitive.
    /// @see
    /// For more information on mutexes, see
    /// [Rust Docs](https://doc.rust-lang.org/std/sync/struct.Mutex.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/timed_mutex.html),
    /// [Wikipedia](https://en.wikipedia.org/wiki/Lock_(computer_science)).
    using mutex = ordinary_mutex<false>;

    /// @ingroup sys_threading
    /// @relates ordinary_mutex
    /// @brief
    /// A variant of the mutex threading primitive, which allows for
    /// @code{.cpp}
    /// auto guard1 = mut.lock();
    /// auto guard2 = mut.lock(); // Ok, imagine secretly incrementing a counter.
    /// @endcode
    /// @see
    /// For more information on mutexes, see
    /// [Rust Docs](https://doc.rust-lang.org/std/sync/struct.Mutex.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/recursive_timed_mutex.html),
    /// [Wikipedia](https://en.wikipedia.org/wiki/Lock_(computer_science)).
    using reentrant_mutex = ordinary_mutex<true>;
} // namespace sys

#undef timespec // NOLINT(misc-include-cleaner)
