// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Alloc>

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("system_allocator<void>, small_buffer_allocator<void, ...>",
                                                                     "[sys.Alloc][system_allocator][small_buffer_allocator]", sys::system_allocator<void>,
                                                                     (sys::small_buffer_allocator<void, 256_uz>))
{
    {
        TestType malloc;

        void* gotMem = malloc.realloc(nullptr, 64_uz /* NOLINT(readability-magic-numbers) */, unsafe);
        CHECK(gotMem);
        malloc.dealloc(gotMem, 64_uz /* NOLINT(readability-magic-numbers) */, unsafe);

        malloc.dealloc(nullptr, 0_uz, unsafe);
    }

    {
        TestType malloc;
        if constexpr (std::same_as<TestType, sys::small_buffer_allocator<void, 256_uz /* NOLINT(readability-magic-numbers) */>>)
        {
            CHECK_FALSE(malloc.alloc(257_uz, unsafe));
            CHECK_FALSE(malloc.alloc(256_uz, sz(alignof(std::max_align_t)) / 2_uz, unsafe));
            CHECK_FALSE(malloc.alloc_zeroed(257_uz, unsafe));
            CHECK_FALSE(malloc.alloc_zeroed(256_uz, sz(alignof(std::max_align_t)) / 2_uz, unsafe));
        }

        void* gotMem = malloc.alloc(128_uz /* NOLINT(readability-magic-numbers) */, unsafe);
        CHECK(gotMem);
        if constexpr (std::same_as<TestType, sys::small_buffer_allocator<void, 256_uz /* NOLINT(readability-magic-numbers) */>>)
        {
            CHECK_FALSE(malloc.alloc(128_uz, unsafe));
            CHECK_FALSE(malloc.realloc(nullptr, 256_uz, unsafe));
            CHECK_FALSE(malloc.realloc(gotMem, 257_uz, unsafe));
            CHECK_FALSE(malloc.realloc(gotMem, 256_uz, sz(alignof(std::max_align_t)) / 2_uz, unsafe));
        }

        void* gotMaybeNullptr = malloc.alloc(128_uz /* NOLINT(readability-magic-numbers) */, unsafe);

        gotMem = malloc.realloc(gotMem, 256_uz /* NOLINT(readability-magic-numbers) */, unsafe);
        CHECK(gotMem);

        malloc.dealloc(gotMem, 256_uz /* NOLINT(readability-magic-numbers) */, unsafe);
        malloc.dealloc(gotMaybeNullptr, 128_uz /* NOLINT(readability-magic-numbers) */, unsafe);
    }
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("system_allocator<T>, small_buffer_allocator<T, ...>", "[sys.Alloc][system_allocator][small_buffer_allocator]",
                                                                     sys::system_allocator<uint_least64_t>, (sys::small_buffer_allocator<uint_least64_t, 128uz>))
{
    TestType malloc;
    uint_least64_t* gotMem = malloc.alloc_zeroed(64_uz /* NOLINT(readability-magic-numbers) */, unsafe);
    uint_least64_t cmp[64uz /* NOLINT(readability-magic-numbers) */] {};

    CHECK(gotMem);
    CHECK(std::memcmp(gotMem, cmp, sz(sizeof(uint_least64_t)) * 64_uz) == 0);

    gotMem = malloc.realloc(gotMem, 128_uz /* NOLINT(readability-magic-numbers) */, unsafe);
    CHECK(gotMem);
    CHECK(std::memcmp(gotMem, cmp, sz(sizeof(uint_least64_t)) * 64_uz) == 0);

    malloc.dealloc(gotMem, 128_uz /* NOLINT(readability-magic-numbers) */, unsafe);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
