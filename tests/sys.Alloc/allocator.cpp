// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Alloc>

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("system_allocator<void>, inplace_allocator<void, ...>, small_buffer_allocator<void, ...>",
                                                                     "[sys.Alloc][system_allocator][inplace_allocator][small_buffer_allocator]", sys::system_allocator<void>,
                                                                     (sys::inplace_allocator<void, 256_uz>), (sys::small_buffer_allocator<void, 128_uz>))
{
    STATIC_CHECK(sys::IAllocator<TestType>); // NOLINT(clang-analyzer-unix.Malloc)

    {
        TestType malloc;

        void* gotMem = malloc.realloc(nullptr, 0_uz, 64_uz /* NOLINT(readability-magic-numbers) */, unsafe);
        CHECK(gotMem);
        malloc.dealloc(gotMem, 64_uz /* NOLINT(readability-magic-numbers) */, unsafe);

        malloc.dealloc(nullptr, 0_uz, unsafe);
    }

    {
        TestType malloc;
        if constexpr (std::same_as<TestType, sys::small_buffer_allocator<void, 256_uz /* NOLINT(readability-magic-numbers) */>>)
        {
            CHECK_FALSE(malloc.alloc(257_uz, unsafe));
            CHECK_FALSE(malloc.alloc_zeroed(257_uz, unsafe));
        }

        CHECK_FALSE(malloc.alloc(256_uz, sz(alignof(std::max_align_t)) * 2_uz, unsafe));
        CHECK_FALSE(malloc.alloc_zeroed(256_uz, sz(alignof(std::max_align_t)) * 2_uz, unsafe));

        void* gotMem = malloc.alloc(128_uz /* NOLINT(readability-magic-numbers) */, unsafe);
        CHECK(gotMem);
        CHECK_FALSE(malloc.realloc(gotMem, 128_uz, sz(alignof(std::max_align_t)), 256_uz, sz(alignof(std::max_align_t)) * 2_uz, unsafe));
        if constexpr (std::same_as<TestType, sys::small_buffer_allocator<void, 256_uz /* NOLINT(readability-magic-numbers) */>>)
        {
            CHECK_FALSE(malloc.alloc(128_uz, unsafe));
            CHECK_FALSE(malloc.realloc(nullptr, 0_uz, 256_uz, unsafe));
            CHECK_FALSE(malloc.realloc(gotMem, 128_uz, 257_uz, unsafe));
        }

        gotMem = malloc.realloc(gotMem, 128_uz, 256_uz /* NOLINT(readability-magic-numbers) */, unsafe);
        CHECK(gotMem);

        void* gotMaybeNullptr = malloc.alloc(128_uz /* NOLINT(readability-magic-numbers) */, unsafe);
        malloc.dealloc(gotMem, 256_uz /* NOLINT(readability-magic-numbers) */, unsafe);
        malloc.dealloc(gotMaybeNullptr, 128_uz /* NOLINT(readability-magic-numbers) */, unsafe);
    }
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("system_allocator<T>, inplace_allocator<T, ...>, small_buffer_allocator<T, ...>",
                                                                     "[sys.Alloc][system_allocator][inplace_allocator][small_buffer_allocator]",
                                                                     sys::system_allocator<uint_least64_t>, (sys::inplace_allocator<uint_least64_t, 256_uz>),
                                                                     (sys::small_buffer_allocator<uint_least64_t, 128_uz>))
{
    TestType malloc;

    CHECK_FALSE(malloc.alloc(sz::highest(), unsafe));
    CHECK_FALSE(malloc.alloc_zeroed(sz::highest(), unsafe));
    CHECK_FALSE(malloc.realloc(nullptr, 0_uz, sz::highest(), unsafe));
    CHECK_FALSE(malloc.alloc(sz::highest(), sz(alignof(std::max_align_t)) * 2_uz, unsafe));
    CHECK_FALSE(malloc.alloc_zeroed(sz::highest(), sz(alignof(std::max_align_t)) * 2_uz, unsafe));
    CHECK_FALSE(malloc.alloc_zeroed(1_uz, sz(alignof(std::max_align_t)) * 2_uz, unsafe));
    CHECK_FALSE(malloc.realloc(nullptr, 0_uz, 1_uz, sz::highest(), sz(alignof(std::max_align_t)) * 2_uz, unsafe));

    uint_least64_t* gotMem = malloc.alloc_zeroed(64_uz /* NOLINT(readability-magic-numbers) */, unsafe);
    uint_least64_t cmp[64uz /* NOLINT(readability-magic-numbers) */] {};

    CHECK(gotMem);
    CHECK(std::memcmp(gotMem, _as(cmp, uint_least64_t*), sz(sizeof(uint_least64_t)) * 64_uz) == 0);

    std::fill(gotMem, gotMem + 64z /* NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic) */, 0x0BADC0DE0BADC0DE /* NOLINT(readability-magic-numbers) */);
    std::fill(cmp, cmp + 64z /* NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay) */, 0x0BADC0DE0BADC0DE /* NOLINT(readability-magic-numbers) */);
    gotMem = malloc.realloc(gotMem, 64_uz, 128_uz /* NOLINT(readability-magic-numbers) */, unsafe);

    CHECK(gotMem);
    CHECK(std::memcmp(gotMem, _as(cmp, uint_least64_t*), sizeof(cmp)) == 0);

    malloc.dealloc(gotMem, 128_uz /* NOLINT(readability-magic-numbers) */, unsafe);
}

TEST_CASE("small_buffer_allocator<...>", "[sys.Alloc][small_buffer_allocator]")
{
    sys::small_buffer_allocator<uint_least64_t, 4uz> malloc;
    uint_least64_t* gotMem = nullptr;

    CHECK((gotMem = malloc.alloc(4_uz, unsafe)));
    malloc.dealloc(gotMem, 4_uz, unsafe);

    CHECK((gotMem = malloc.alloc(8_uz, unsafe) /* NOLINT(clang-analyzer-unix.Malloc) */));
    malloc.dealloc(gotMem, 8_uz /* NOLINT(readability-magic-numbers) */, unsafe);

    CHECK((gotMem = malloc.alloc_zeroed(4_uz, unsafe)));
    malloc.dealloc(gotMem, 4_uz, unsafe);

    CHECK((gotMem = malloc.alloc_zeroed(8_uz, unsafe) /* NOLINT(clang-analyzer-unix.Malloc) */));
    malloc.dealloc(gotMem, 8_uz /* NOLINT(readability-magic-numbers) */, unsafe);

    CHECK((gotMem = malloc.realloc(nullptr, 0_uz, 4_uz, unsafe)));
    malloc.dealloc(gotMem, 4_uz, unsafe);

    CHECK((gotMem = malloc.realloc(nullptr, 0_uz, 8_uz, unsafe) /* NOLINT(clang-analyzer-unix.Malloc) */));
    malloc.dealloc(gotMem, 8_uz /* NOLINT(readability-magic-numbers) */, unsafe);

    CHECK((gotMem = malloc.alloc(4_uz, unsafe)));
    CHECK((gotMem = malloc.realloc(gotMem, 4_uz, 8_uz, unsafe) /* NOLINT(clang-analyzer-unix.Malloc) */));
    malloc.dealloc(gotMem, 8_uz /* NOLINT(readability-magic-numbers) */, unsafe);

    CHECK((gotMem = malloc.alloc(8_uz, unsafe) /* NOLINT(clang-analyzer-unix.Malloc) */));
    CHECK((gotMem = malloc.realloc(gotMem, 8_uz, 4_uz, unsafe)));
    malloc.dealloc(gotMem, 4_uz, unsafe);

    CHECK((gotMem = malloc.alloc(4_uz, unsafe)));
    CHECK((gotMem = malloc.realloc(gotMem, 4_uz, 4_uz, unsafe)));
    malloc.dealloc(gotMem, 4_uz, unsafe);

    CHECK((gotMem = malloc.alloc(8_uz, unsafe) /* NOLINT(clang-analyzer-unix.Malloc) */));
    CHECK((gotMem = malloc.realloc(gotMem, 8_uz, 8_uz, unsafe) /* NOLINT(clang-analyzer-unix.Malloc) */));
    malloc.dealloc(gotMem, 8_uz /* NOLINT(readability-magic-numbers) */, unsafe);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
