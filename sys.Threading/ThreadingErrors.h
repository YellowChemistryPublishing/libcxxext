#pragma once

/// @file

#include <LanguageSupport.h>

namespace sys
{
    /// @ingroup sys_threading
    enum class threading_error : byte
    {
        ok = 0,
        oom,
        init_failed,
        not_initialized,
        operation_failed,
        busy,
        invalid_argument,
        invalid_operation,
        overflow
    };
}; // namespace sys
