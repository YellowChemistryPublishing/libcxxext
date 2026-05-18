#pragma once

/// @file

#define NOMINMAX 1 // NOLINT(readability-identifier-naming)

#include <cstddef>
#include <tinycthread.h>

#undef NOMINMAX // NOLINT(misc-include-cleaner): Spurious.
#ifdef call_once
#undef call_once
#endif

#include <LanguageSupport.h>
#include <sup/ThreadingErrors.h>

namespace sys::internal
{
    struct cond_var_handle;

    /// @internal
    /// @ingroup sys_internal
    struct mutex_handle
    {
    private:
        mtx_t mut {};
    public:
        /* NOLINT(hicpp-explicit-conversions) */ mutex_handle(std::nullptr_t) noexcept { }
        mutex_handle(const mutex_handle&) noexcept = delete;
        mutex_handle(mutex_handle&&) noexcept = delete;
        ~mutex_handle() noexcept = default;

        mutex_handle& operator=(const mutex_handle&) noexcept = delete;
        mutex_handle& operator=(mutex_handle&&) noexcept = delete;

        template <bool IsRecursive>
        internal::threading_error create() noexcept
        {
            // TODO(halloimdragon): `mtx_timed` is bugged by the way, and hangs your program easily!!!! Don't add!!
            return _as(mtx_init(&this->mut, _as(mtx_plain, unsigned) | _as(IsRecursive ? mtx_recursive : 0, unsigned)), internal::threading_error);
        }
        void destroy() noexcept { mtx_destroy(&this->mut); }
        internal::threading_error try_lock() noexcept { return _as(mtx_trylock(&this->mut), internal::threading_error); }
        internal::threading_error lock() noexcept { return _as(mtx_lock(&this->mut), internal::threading_error); }
        internal::threading_error unlock() noexcept { return _as(mtx_unlock(&this->mut), internal::threading_error); }

        friend struct sys::internal::cond_var_handle;
    };
} // namespace sys::internal
