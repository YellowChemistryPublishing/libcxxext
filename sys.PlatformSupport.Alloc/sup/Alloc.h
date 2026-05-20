#pragma once

/// @file

#include <cstdlib>

#include <Integer.h>
#include <LanguageSupport.h>

namespace sys::internal
{
    /// @internal
    /// @ingroup sys_internal
    inline void* global_alloc(const sz size, [[maybe_unused]] const sz align, decltype(unsafe)) noexcept
    {
        return std::malloc(size) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */;
    }
    /// @internal
    /// @ingroup sys_internal
    inline void* global_alloc_zeroed(const sz size, [[maybe_unused]] const sz align, decltype(unsafe)) noexcept
    {
        return std::calloc(size, 1uz) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */;
    }
    /// @internal
    /// @ingroup sys_internal
    inline void* global_realloc(void* ptr, [[maybe_unused]] const sz size, [[maybe_unused]] const sz align, const sz newSize, [[maybe_unused]] const sz newAlign,
                                decltype(unsafe)) noexcept
    {
        return std::realloc(ptr, newSize) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */;
    }
    /// @internal
    /// @ingroup sys_internal
    inline void global_dealloc(void* ptr, [[maybe_unused]] const sz size, [[maybe_unused]] const sz align, decltype(unsafe)) noexcept
    {
        std::free(ptr) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */;
    }
} // namespace sys::internal
