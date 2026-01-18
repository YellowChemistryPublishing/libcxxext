#pragma once

#include <rt_threading.h>

namespace sys
{
    using thread_id = ::sys::platform::thread_id;

    using thread_critical_section_isr = ::sys::platform::thread_critical_section_isr;
    using thread_critical_section = ::sys::platform::thread_critical_section;

    using thread = ::sys::platform::thread_handle;
} // namespace sys
