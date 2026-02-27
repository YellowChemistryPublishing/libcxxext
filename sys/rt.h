#pragma once

/// @file rt.h

#include <cstddef>

namespace sys::platform
{
    /// @internal
    /// @brief Runtime support function to allocate memory for a `sys::task`.
    extern "C" void* task_operator_new(size_t);
    /// @internal
    /// @brief Runtime support function to deallocate memory for a `sys::task`.
    extern "C" void task_operator_delete(void*);

    /// @internal
    /// @brief Runtime support function to launch a `sys::async`.
    extern "C" void launch_async(void*);
} // namespace sys::platform
