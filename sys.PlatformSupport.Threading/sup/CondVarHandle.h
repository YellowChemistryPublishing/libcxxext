#pragma once

/// @file

#include <CompilerWarnings.h>
#define NOMINMAX 1 // NOLINT(readability-identifier-naming)
_nowarn_begin_one_clang(_clwarn_clang_documentation);

#include <cstddef>
#include <tinycthread.h>

_nowarn_end_clang();
#undef NOMINMAX // NOLINT(misc-include-cleaner): Spurious.
#ifdef call_once
#undef call_once
#endif

#include <LanguageSupport.h>
#include <sup/ThreadingErrors.h>

#if !defined(_libcxxext_internal_mock_sup_mut) || !_libcxxext_internal_mock_sup_mut
#include <sup/MutexHandle.h>
#endif

namespace sys::internal
{
    /// @internal
    /// @ingroup sys_internal
    struct cond_var_handle
    {
    private:
        cnd_t cond {};
    public:
        /* NOLINT(hicpp-explicit-conversions) */ cond_var_handle(std::nullptr_t) noexcept { }
        cond_var_handle(const cond_var_handle&) noexcept = delete;
        cond_var_handle(cond_var_handle&&) noexcept = delete;
        ~cond_var_handle() noexcept = default;

        cond_var_handle& operator=(const cond_var_handle&) noexcept = delete;
        cond_var_handle& operator=(cond_var_handle&&) noexcept = delete;

        internal::threading_error create() noexcept { return _as(cnd_init(&this->cond), internal::threading_error); }
        void destroy() noexcept { cnd_destroy(&this->cond); }
        internal::threading_error signal() noexcept { return _as(cnd_signal(&this->cond), internal::threading_error); }
        internal::threading_error broadcast() noexcept { return _as(cnd_broadcast(&this->cond), internal::threading_error); }
        internal::threading_error wait(internal::mutex_handle& mut) noexcept { return _as(cnd_wait(&this->cond, &mut.mut), internal::threading_error); }
    };
} // namespace sys::internal
