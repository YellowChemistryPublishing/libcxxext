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

namespace sys::internal
{
    struct thread_handle thread_current(decltype(unsafe)) noexcept;

    /// @internal
    /// @ingroup sys_internal
    struct thread_handle
    {
    private:
        thrd_t th {};

        thread_handle(thrd_t th, decltype(unsafe)) noexcept : th(th) { }
    public:
        /* NOLINT(hicpp-explicit-conversions) */ thread_handle(std::nullptr_t) { }
        thread_handle(const thread_handle&) noexcept = default;
        thread_handle(thread_handle&&) noexcept = default;
        ~thread_handle() noexcept = default;

        thread_handle& operator=(const thread_handle&) noexcept = default;
        thread_handle& operator=(thread_handle&&) noexcept = default;

        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        friend bool operator==(const thread_handle& a, const thread_handle& b) noexcept { return thrd_equal(a.th, b.th); }

        [[nodiscard]] internal::threading_error create(int (*func)(void*), void* arg, decltype(unsafe)) noexcept
        {
            return _as(thrd_create(&this->th, func, arg), internal::threading_error);
        }
        [[nodiscard]] internal::threading_error join(int* res, decltype(unsafe)) const noexcept { return _as(thrd_join(this->th, res), internal::threading_error); }
        [[nodiscard]] internal::threading_error detach(decltype(unsafe)) const noexcept { return _as(thrd_detach(this->th), internal::threading_error); }

        friend thread_handle sys::internal::thread_current(decltype(unsafe)) noexcept;
    };

    /// @internal
    /// @ingroup sys_internal
    [[nodiscard]] inline thread_handle thread_current(decltype(unsafe)) noexcept { return { thrd_current(), unsafe }; }
    /// @internal
    /// @ingroup sys_internal
    inline void thread_yield(decltype(unsafe)) noexcept { thrd_yield(); }
} // namespace sys::internal
