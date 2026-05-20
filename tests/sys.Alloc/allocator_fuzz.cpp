#include <array>
#include <cstdlib>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);
_nowarn_begin_deprecated();
_nowarn_begin_conv_comp();
_nowarn_begin_unreachable();

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_nowarn_end_unreachable();
_nowarn_end_conv_comp();
_nowarn_end_deprecated();
_nowarn_end_gcc();

#include <module/sys>

namespace sys::internal
{
    // NOLINTBEGIN(concurrency-mt-unsafe, misc-predictable-rand): I do not care.
    static inline void* global_alloc(const sz size, [[maybe_unused]] const sz align, decltype(unsafe)) noexcept
    {
        return std::rand() % 2 == 0 ? std::malloc(size) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */ : nullptr;
    }
    static inline void* global_alloc_zeroed(const sz size, [[maybe_unused]] const sz align, decltype(unsafe)) noexcept
    {
        return std::rand() % 2 == 0 ? std::calloc(size, 1uz) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */ : nullptr;
    }
    static inline void* global_realloc(void* ptr, [[maybe_unused]] const sz size, [[maybe_unused]] const sz align, const sz newSize, [[maybe_unused]] const sz newAlign,
                                       decltype(unsafe)) noexcept
    {
        return std::rand() % 2 == 0 ? std::realloc(ptr, newSize) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */ : nullptr;
    }
    static inline void global_dealloc(void* ptr, [[maybe_unused]] const sz size, [[maybe_unused]] const sz align, decltype(unsafe)) noexcept
    {
        std::free(ptr) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */;
    }
    // NOLINTEND(concurrency-mt-unsafe, misc-predictable-rand)
} // namespace sys::internal

#define _libcxxext_internal_mock_sup_alloc 1
#include <module/sys.Alloc>

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("[[fuzz]] system_allocator<...>, small_buffer_allocator<...>, dynamic_allocator<...>",
                                                                     "[fuzz][sys.Alloc][system_allocator][small_buffer_allocator][dynamic_allocator]",
                                                                     (sys::system_allocator<uint_least16_t>),
                                                                     (sys::small_buffer_allocator<uint_least16_t, *sz(u16::highest() / 2_u8)>),
                                                                     (sys::dynamic_allocator<uint_least16_t, *sz(u16::highest() / 2_u8)>))
{
    CHECK(rc::check([](const std::array<uint_least16_t, 2uz>& a) -> void
    {
        TestType malloc;
        uint_least16_t* gotMem = nullptr;

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        gotMem = std::rand() % 2 == 0 /* NOLINT(concurrency-mt-unsafe, misc-predictable-rand) */ ? malloc.alloc(sz(a[0uz]), unsafe) : malloc.alloc_zeroed(sz(a[0uz]), unsafe);
        gotMem = malloc.realloc(gotMem, sz(a[0uz]), sz(a[1uz]), unsafe);
        malloc.dealloc(gotMem, sz(a[1uz]), unsafe) /* NOLINT(clang-analyzer-unix.Malloc) */;
        // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    }));
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
