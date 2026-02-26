#pragma once

/// @file Thread.h

#include <rt_threading.h>

namespace sys
{
    using thread_id = ::sys::platform::thread_id; /**< Numerical thread ID. */

    using thread_critical_section_isr = ::sys::platform::thread_critical_section_isr; /**< RAII guard type for ISR-safe critical section. */
    using thread_critical_section = ::sys::platform::thread_critical_section;         /**< RAII guard type for critical section. */

    using thread = ::sys::platform::thread_handle; /**< Thread handle type. */
} // namespace sys
