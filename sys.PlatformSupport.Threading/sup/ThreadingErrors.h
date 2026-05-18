#pragma once

/// @file

#define NOMINMAX 1 // NOLINT(readability-identifier-naming)
#include <tinycthread.h>
#undef NOMINMAX // NOLINT(misc-include-cleaner): Spurious.
#ifdef call_once
#undef call_once
#endif

#include <LanguageSupport.h>

namespace sys::internal
{
    enum class threading_error : byte
    {
        error = thrd_error,
        ok = thrd_success,
        timeout = thrd_timedout,
        busy = thrd_busy,
        oom = thrd_nomem
    };
} // namespace sys::internal
