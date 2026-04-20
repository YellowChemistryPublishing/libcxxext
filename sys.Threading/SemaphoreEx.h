#pragma once

/// @file

#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include <ConditionVariable.h>
#include <LanguageSupport.h>
#include <Mutex.h>
#include <ResourceGuard.h>
#include <Result.h>
#include <ThreadingErrors.h>
#include <inline/Integer.inl>
#include <meta/Builtin.h>

namespace sys
{
    /// @ingroup sys_threading
    /// @brief A semaphore threading primitive.
    /// @tparam DefaultConcurrentAccessors A statically-known default number of concurrent accesses, or `sys::bsentinel<T>()` to require a constructor argument.
    /// @details Implements `sys::INothrowDestructible`. Conditionally implements `sys::INothrowDefaultConstructible` when `DefaultConcurrentAccessors != sys::bsentinel<T>()`.
    template <sys::IBuiltinInteger T, T DefaultConcurrentAccessors = sys::bsentinel<T>()>
    requires (DefaultConcurrentAccessors >= _as(T, 0) || DefaultConcurrentAccessors == sys::bsentinel<T>())
    class [[clang::capability("semaphore")]] ordinary_semaphore final
    {
    private:
        sys::mutex mut;
        sys::cond_var cv;
        T counter = DefaultConcurrentAccessors;
    public:
        ordinary_semaphore() noexcept
        requires (DefaultConcurrentAccessors != sys::bsentinel<T>())
        = default;
        /// @brief Constructs a new semaphore with the given initial count.
        /// @pre `init_count >= 0`.
        /// @warning `unsafe` because `this` has preconditions.
        constexpr explicit ordinary_semaphore(const sys::integer<T> init_count, decltype(unsafe)) noexcept /* NOLINT(bugprone-exception-escape) */
        requires (DefaultConcurrentAccessors == sys::bsentinel<T>())
            : counter(init_count)
        {
            if consteval
            {
                throw std::domain_error("`init_count` must be larger than or equal to `0`.");
            }
            else
            {
                _contract_assert(init_count >= 0);
            }
        }
        ordinary_semaphore(const ordinary_semaphore&) noexcept = delete;
        ordinary_semaphore(ordinary_semaphore&&) noexcept = delete;
        ~ordinary_semaphore() noexcept = default;

        ordinary_semaphore& operator=(const ordinary_semaphore&) noexcept = delete;
        ordinary_semaphore& operator=(ordinary_semaphore&&) noexcept = delete;

        /// @brief Acquires a permit.
        /// @post There must exist a corresponding, subsequent call to `.release(...)`, within a finite amount of time.
        /// @warning `unsafe` because `this` has postconditions.
        /// @return Propagated error from `sys::mutex::lock()`, `sys::cond_var::wait_until(...)`, or success.
        sys::result<void, threading_error> acquire(decltype(unsafe)) noexcept
        {
            auto guardRes = this->mut.lock();
            _retif(guardRes.err(), !guardRes);
            _retif(waitRes.err(), auto waitRes = this->cv.wait_until(this->mut, [&]() -> bool { return this->counter > _as(T, 0); }); !waitRes);

            --this->counter;
            return {};
        }
        /// @brief Releases a permit.
        /// @pre There must exist a corresponding, preceding call to `.acquire(...)`.
        /// @warning `unsafe` because `this` has preconditions.
        /// @return Propagated error from `sys::mutex::lock()`, `sys::cond_var::notify_one()`, or `threading_error::overflow` if the semaphore is already at its maximum capacity,
        /// otherwise success.
        sys::result<void, threading_error> release(decltype(unsafe)) noexcept
        {
            auto guardRes = this->mut.lock();
            _retif(guardRes.err(), !guardRes);
            _retif(threading_error::overflow, this->counter == *sys::integer<T>::highest());

            ++this->counter;
            if (auto notifRes = this->cv.notify_one(); !notifRes) [[unlikely]]
            {
                --this->counter;
                return notifRes.err();
            }

            return {};
        }
    private:
        static void release_guard(ordinary_semaphore* sem) noexcept { _contract_assert(!sem || sem->release(unsafe), "If this happens we're genuinely cooked."); }
    public:
        using guard = sys::resource_guard<ordinary_semaphore, &ordinary_semaphore::release_guard>;

        /// @brief Attempts to acquire a permit.
        /// @return The error from `sys::ordinary_semaphore::acquire(...)`, or a guard on success.
        sys::result<guard, threading_error> access() noexcept
        {
            auto acqRes = this->acquire(unsafe);
            _retif(acqRes.err(), !acqRes);
            return guard(*this, unsafe);
        }
    };

    /// @ingroup sys_threading
    /// @relates ordinary_semaphore
    /// @brief A binary semaphore, initialized with one permit.
    /// @see
    /// For more information on semaphore threading primitives, see
    /// [Rust Docs](https://docs.rs/semaphore/latest/semaphore/struct.Semaphore.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/counting_semaphore.html).
    using binary_semaphore = ordinary_semaphore<uint_least8_t, _as(uint_least8_t, 1)>;

    /// @ingroup sys_threading
    /// @relates ordinary_semaphore
    /// @brief A semaphore with a runtime-specified initial count of `size_t`.
    /// @see
    /// For more information on semaphore threading primitives, see
    /// [Rust Docs](https://docs.rs/semaphore/latest/semaphore/struct.Semaphore.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/counting_semaphore.html).
    using semaphore = ordinary_semaphore<size_t, sys::bsentinel<size_t>()>;
} // namespace sys
