#pragma once

/// @file

#define NOMINMAX 1 // NOLINT(readability-identifier-naming)
#include <tinycthread.h>
#undef NOMINMAX
#ifdef call_once
#undef call_once
#endif

#include <LanguageSupport.h>
#include <Once.h>
#include <Result.h>
#include <ThreadingErrors.h>
#include <meta/Type.h>

namespace sys
{
    template <bool>
    class ordinary_mutex;

    /// @brief Condition variable threading primitive.
    /// @details Implements `std::is_nothrow_default_constructible_v`, `std::is_nothrow_destructible_v`.
    /// @see
    /// For more information on condition variables, see
    /// [Rust Docs](https://doc.rust-lang.org/std/sync/struct.Condvar.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/condition_variable).
    class cond_var final
    {
        cnd_t cond {};
        once o;

        sys::result<void, threading_error> try_init() noexcept
        {
            _retif(threading_error::init_failed,
                   !this->o.call_once([&]() noexcept -> sys::result<void>
            {
                _retif(nullptr, cnd_init(&this->cond) != thrd_success);
                return {};
            }));
            return {};
        }
    public:
        cond_var() noexcept = default;
        cond_var(const cond_var&) noexcept = delete;
        cond_var(cond_var&&) noexcept = delete;
        ~cond_var() noexcept
        {
            if (this->o.is_completed()) [[likely]]
                cnd_destroy(&this->cond);
        }

        cond_var& operator=(const cond_var&) noexcept = delete;
        cond_var& operator=(cond_var&&) noexcept = delete;

        /// @brief Wait for the condition variable to be notified.
        /// @pre `mut` must be locked, and locked by the calling thread.
        /// @warning
        /// You should note that `sys::cond_var` is allowed to spuriously awaken.
        /// Be _very_ careful if you choose to wait with a `sys::reentrant_mutex`.
        template <typename T>
        requires (sys::meta::type<T>::template is_from<ordinary_mutex>())
        sys::result<void, threading_error> wait(T& mut) noexcept
        {
            auto initRes = this->try_init();
            _retif(initRes, !initRes);
            _retif(threading_error::operation_failed, cnd_wait(&this->cond, &mut.mut) != thrd_success);
            return {};
        }
        /// @brief Wait for the condition variable to be notified, until `pred()` is `true`.
        /// @pre `mut` must be locked, and locked by the calling thread.
        /// @details Equivalent to
        /// ```cpp
        /// while (!pred())
        /// { /* wait ... */ }
        /// ```
        /// @warning Be _very_ careful if you choose to wait with a `sys::reentrant_mutex`.
        template <typename T, typename Pred>
        requires (sys::meta::type<T>::template is_from<ordinary_mutex>())
        sys::result<void, threading_error> wait_until(T& mut, Pred&& pred) noexcept
        {
            auto initRes = this->try_init();
            _retif(initRes, !initRes);
            while (!std::forward<Pred>(pred)())
            {
                auto waitRes = this->wait(mut);
                _retif(waitRes, !waitRes);
            }
            return {};
        }
        // TODO(halloimdragon): `wait_timeout(...)`.

        /// @brief Notify one thread waiting on this condition variable.
        sys::result<void, threading_error> notify_one() noexcept
        {
            auto initRes = this->try_init();
            _retif(initRes, !initRes);
            _retif(threading_error::operation_failed, cnd_signal(&this->cond) != thrd_success);
            return {};
        }
        /// @brief Notify all threads waiting on this condition variable.
        sys::result<void, threading_error> notify_all() noexcept
        {
            auto initRes = this->try_init();
            _retif(initRes, !initRes);
            _retif(threading_error::operation_failed, cnd_broadcast(&this->cond) != thrd_success);
            return {};
        }
    };
} // namespace sys
