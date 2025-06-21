#pragma once

#include <cstddef>

namespace sys::platform
{
    extern "C" void* _task_operator_new(size_t);
    extern "C" void _task_operator_delete(void*);

    extern "C" void _launch_async(void*);
} // namespace sys::platform
