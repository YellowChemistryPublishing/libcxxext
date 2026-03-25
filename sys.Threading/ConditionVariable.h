#pragma once

#include <tinycthread.h>

#include <LanguageSupport.h>
#include <Once.h>
#include <Result.h>
#include <ThreadingErrors.h>
#include <meta/Type.h>

namespace sys
{
    template <bool>
    class ordinary_mutex;

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

        /// @warning Be _very_ careful if you choose to wait with a `sys::reentrant_mutex`.
        template <typename T>
        requires (sys::meta::type<T>::template is_from<ordinary_mutex>())
        sys::result<void, threading_error> wait(T& mut) noexcept
        {
            auto initRes = this->try_init();
            _retif(initRes, !initRes);
            _retif(threading_error::operation_failed, cnd_wait(&this->cond, &mut.mut) != thrd_success);
            return {};
        }
        // TODO(halloimdragon): `wait_while(...)`, `wait_timeout(...)`.

        sys::result<void, threading_error> notify_one() noexcept
        {
            auto initRes = this->try_init();
            _retif(initRes, !initRes);
            _retif(threading_error::operation_failed, cnd_signal(&this->cond) != thrd_success);
            return {};
        }
        sys::result<void, threading_error> notify_all() noexcept
        {
            auto initRes = this->try_init();
            _retif(initRes, !initRes);
            _retif(threading_error::operation_failed, cnd_broadcast(&this->cond) != thrd_success);
            return {};
        }
    };
} // namespace sys
