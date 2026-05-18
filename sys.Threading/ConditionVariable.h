#pragma once

/// @file

#include <concepts>
#include <type_traits>

#include <CompilerWarnings.h>
#include <LanguageSupport.h>
#include <Once.h>
#include <Result.h>
#include <ThreadingErrors.h>
#include <meta/InterfaceRequirements.h>
#include <meta/NamedRequirements.h>
#include <meta/Type.h>

#include <sup/ThreadingErrors.h>

#if !defined(_libcxxext_mock_sup_cv) || !_libcxxext_mock_sup_cv
#include <sup/CondVarHandle.h>
#endif

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
        internal::cond_var_handle cond = nullptr;
        once o;

        sys::result<void> try_init() noexcept
        {
            return this->o.call_once([&]() noexcept -> sys::result<void>
            {
                _retif(nullptr, this->cond.create() != internal::threading_error::ok);
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
                this->cond.destroy();
        }

        cond_var& operator=(const cond_var&) noexcept = delete;
        cond_var& operator=(cond_var&&) noexcept = delete;

        /// @brief Wait for the condition variable to be notified.
        /// @pre `mut` must be locked, and locked by the calling thread.
        /// @warning
        /// You should note that `sys::cond_var` is allowed to spuriously awaken.
        /// Be _very_ careful if you choose to wait with a `sys::reentrant_mutex`.
        [[nodiscard]] sys::result<void, threading_error> wait(auto& mut) noexcept
        requires (sys::meta::type<_decltype_of(mut)>::template is_from<ordinary_mutex>())
        {
            _retif(threading_error::init_failed, !this->try_init());
            _retif(threading_error::operation_failed, this->cond.wait(mut.mut) != internal::threading_error::ok);
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
        [[nodiscard]] sys::result<void, threading_error> wait_until(auto& mut, ICallable auto&& pred) noexcept(INothrowCallable<decltype(pred)>)
        requires requires {
            requires sys::meta::type<_decltype_of(mut)>::template is_from<ordinary_mutex>();
            requires std::convertible_to<std::invoke_result_t<_decltype_of(pred)>, bool>;
        }
        {
            while (!_forward(pred)() /* NOLINT(bugprone-use-after-move): Spurious. */)
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
            _retif(threading_error::operation_failed, this->cond.signal() != internal::threading_error::ok);
            return {};
        }
        /// @brief Notify all threads waiting on this condition variable.
        [[nodiscard]] sys::result<void, threading_error> notify_all() noexcept
        {
            _retif(threading_error::init_failed, !this->try_init());
            _retif(threading_error::operation_failed, this->cond.broadcast() != internal::threading_error::ok);
            return {};
        }
    };
} // namespace sys
