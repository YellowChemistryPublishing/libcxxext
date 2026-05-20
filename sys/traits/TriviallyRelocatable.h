#pragma once

/// @file

#include <meta/Type.h>

namespace sys::traits
{
    /// @ingroup sys
    /// @brief Tag type to mark directly derived types as trivially relocatable.
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    struct trivially_relocatable
    { };
} // namespace sys::traits
