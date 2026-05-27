#pragma once

/// @file

#include <LanguageSupport.h>

namespace sys
{
    enum class container_error : byte
    {
        ok,
        oom,
        overflow,
        out_of_range,
    };
}