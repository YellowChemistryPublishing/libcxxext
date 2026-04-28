#pragma once

/// @file

#include <CompilerWarnings.h>

#define NOMINMAX 1 // NOLINT(readability-identifier-naming)
#include <concepts>
_nowarn_begin_one_clang(_clwarn_clang_documentation);
#include <tinycthread.h>
_nowarn_end_clang();
#undef NOMINMAX // NOLINT(misc-include-cleaner): Spurious.
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

    /// @ingroup sys_threading
    /// @brief Condition variable threading primitive.
    /// @details Implements `sys::INothrowDefaultConstructible` and `sys::INothrowDestructible`.
    /// @see
    /// For more information on condition variables, see
    /// [Rust Docs](https://doc.rust-lang.org/std/sync/struct.Condvar.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/condition_variable).
    class cond_var final
    {
        cnd_t cond {};
        once o;

        sys::result<void> try_init() noexcept
        {
            return this->o.call_once([&]() noexcept -> sys::result<void>
            {
                _retif(nullptr, cnd_init(&this->cond) != thrd_success);
                return {};
            });
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
        [[nodiscard]] sys::result<void, threading_error> wait(T& mut) noexcept
        {
            _retif(threading_error::init_failed, !this->try_init());
            _retif(threading_error::operation_failed, cnd_wait(&this->cond, &mut.mut) != thrd_success);
            return {};
        }
        /// @brief Wait for the condition variable to be notified, until `pred()` is `true`.
        /// @pre `mut` must be locked, and locked by the calling thread.
        /// @details Equivalent to
        /// @code{.cpp}
        /// while (!pred())
        /// { /* wait ... */ }
        /// @endcode
        /// @warning Be _very_ careful if you choose to wait with a `sys::reentrant_mutex`.
        /// @see `sys::cond_var::wait(T&)`
        template <typename T, typename Pred>
        [[nodiscard]] sys::result<void, threading_error> wait_until(T& mut, Pred&& pred) noexcept(noexcept(pred()))
        requires requires {
            requires sys::meta::type<T>::template is_from<ordinary_mutex>();
            { std::forward<Pred>(pred)() } -> std::convertible_to<bool>;
        }
        {
            _retif(threading_error::init_failed, !this->try_init());
            while (!std::forward<Pred>(pred)())
            {
                auto waitRes = this->wait(mut);
                _nowarn_begin_one_clang(_clwarn_clang_consumed);
                _retif(waitRes, !waitRes);
                _nowarn_end_clang();
            }
            return {};
        }
        // TODO(halloimdragon): `wait_timeout(...)`.

        /// @brief Notify one thread waiting on this condition variable.
        [[nodiscard]] sys::result<void, threading_error> notify_one() noexcept
        {
            _retif(threading_error::init_failed, !this->try_init());
            _retif(threading_error::operation_failed, cnd_signal(&this->cond) != thrd_success);
            return {};
        }
        /// @brief Notify all threads waiting on this condition variable.
        [[nodiscard]] sys::result<void, threading_error> notify_all() noexcept
        {
            _retif(threading_error::init_failed, !this->try_init());
            _retif(threading_error::operation_failed, cnd_broadcast(&this->cond) != thrd_success);
            return {};
        }
    };
} // namespace sys
