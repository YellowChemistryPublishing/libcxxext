#pragma once

/// @file

#define NOMINMAX 1 // NOLINT(readability-identifier-naming)
#include <tinycthread.h>
#undef NOMINMAX // NOLINT(misc-include-cleaner): Spurious.
#ifdef call_once
#undef call_once
#endif

#include <LanguageSupport.h>
#include <Once.h>
#include <ResourceGuard.h>
#include <Result.h>
#include <ThreadingErrors.h>

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
        mtx_t mut {};
        once o;

        result<void> try_init() noexcept
        {
            return this->o.call_once([&]() noexcept -> sys::result<void>
            {
                _retif(nullptr, mtx_init(&this->mut, _as(unsigned, mtx_plain) | _as(unsigned, mtx_timed) | _as(unsigned, IsRecursive ? mtx_recursive : 0)) != thrd_success);
                return {};
            });
        }

        /// @brief Run a possibly-failing, acquire routine.
        /// @pre `this->try_init() && this->o.is_completed()`
        /// @warning `unsafe` because `this` has preconditions.
        template <int (*Acquire)(mtx_t*)>
        bool acq(int& ret, decltype(unsafe)) noexcept
        {
            return (ret = Acquire(&this->mut)) == thrd_success;
        }
    public:
        ordinary_mutex() noexcept = default;
        ordinary_mutex(const ordinary_mutex&) noexcept = delete;
        ordinary_mutex(ordinary_mutex&&) noexcept = delete;
        ~ordinary_mutex() noexcept
        {
            if (this->o.is_completed()) [[likely]]
                mtx_destroy(&this->mut);
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
            int ret = thrd_success;
            _retif(threading_error::init_failed, !this->try_init());
            _retif(threading_error::operation_failed, !this->acq<&mtx_lock>(ret, unsafe));
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
            _retif(threading_error::init_failed, !this->try_init());

            int ret = thrd_success;
            _retif(threading_error::busy, !this->acq<&mtx_trylock>(ret, unsafe) && ret == thrd_busy);
            _retif(threading_error::operation_failed, ret != thrd_success);
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
        result<void, threading_error> release(decltype(unsafe)) noexcept
        {
            _contract_assert(this->o.is_completed(), "`.acquire()` never called!");
            _retif(threading_error::operation_failed, mtx_unlock(&this->mut) != thrd_success);
            return {};
        }
    private:
        static void release_guard(ordinary_mutex* m) noexcept { _contract_assert(!m || m->release(unsafe), "If this happens we're genuinely cooked."); };
    public:
        /// @brief RAII guard for `sys::ordinary_mutex`.
        /// @details
        /// Implements `sys::INothrowDefaultConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`, `sys::INothrowSwappable`.
        using guard = resource_guard<ordinary_mutex, &ordinary_mutex::release_guard>;

        /// @brief Lock and obtain a lock guard for this mutex.
        result<guard, threading_error> lock() noexcept
        {
            _retif(threading_error::init_failed, !this->try_init());

            int ret = thrd_success;
            _retif(threading_error::operation_failed, !this->acq<&mtx_lock>(ret, unsafe));
            return guard(*this, unsafe);
        }
        /// @brief Try to lock this mutex and obtain a lock guard for it, if possible.
        result<guard, threading_error> try_lock() noexcept
        {
            _retif(threading_error::init_failed, !this->try_init());

            int ret = thrd_success;
            _retif(threading_error::busy, !this->acq<&mtx_trylock>(ret, unsafe) && ret == thrd_busy);
            _retif(threading_error::operation_failed, ret != thrd_success);
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
