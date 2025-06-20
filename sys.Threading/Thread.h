#pragma once

#include <rt_threading.h>

namespace sys
{
    using thread_id = __thread_id;

    using thread_critical_section_isr = __tcs_isr;
    using thread_critical_section = __tcs;

    using thread = __thread_type;
} // namespace sys
