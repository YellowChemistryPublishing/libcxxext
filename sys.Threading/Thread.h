#pragma once

#include <rt_threading.h>

namespace sys
{
    using thread_id = ::sys::platform::ThreadID;

    using thread_critical_section_isr = ::sys::platform::ThreadCriticalSectionISR;
    using thread_critical_section = ::sys::platform::ThreadCriticalSection;

    using thread = ::sys::platform::ThreadHandle;
} // namespace sys
