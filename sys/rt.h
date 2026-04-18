#pragma once

/// @file

#include <cstddef>

/// @namespace sys::platform
/// @ingroup sys_internal
/// @brief Platform-specific functionality.

namespace sys::platform
{
    /// @internal
    /// @ingroup sys_internal
    /// @brief Runtime support function to allocate memory for a `sys::task`.
    extern "C" void* task_operator_new(size_t);
    /// @internal
    /// @ingroup sys_internal
    /// @brief Runtime support function to deallocate memory for a `sys::task`.
    extern "C" void task_operator_delete(void*);

    /// @internal
    /// @ingroup sys_internal
    /// @brief Runtime support function to launch a `sys::async`.
    extern "C" void launch_async(void*);
} // namespace sys::platform
