#pragma once

#include <cstddef>

namespace sys::platform
{
    /// @brief Runtime support function to allocate memory for a `sys::task`.
    extern "C" void* task_operator_new(size_t);
    /// @brief Runtime support function to deallocate memory for a `sys::task`.
    extern "C" void task_operator_delete(void*);

    /// @brief Runtime support function to launch a `sys::async`.
    extern "C" void launch_async(void*);
} // namespace sys::platform
