#pragma once

/// @file

#include <Numeric.h>

namespace sys
{
    enum class threading_error : byte
    {
        ok = 0,
        oom,
        init_failed,
        operation_failed,
        invalid_argument,
        invalid_operation
    };
}; // namespace sys
