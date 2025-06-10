#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <cstring>
#include <new>

#define class struct
#define private public
#include <Allocator.h>
#undef private
#undef class

TEST_CASE("basic | `sys::allocator<int, 16_i16, true>`")
{
    sys::allocator<int, 16_i16, true> alloc;

    int* allocated = alloc.allocate(24u);
    CHECK((_as(void*, allocated) < _as(void*, &alloc) || _as(void*, allocated) > _as(void*, &alloc + 1)));
    alloc.deallocate(allocated, 24u);

    allocated = alloc.allocate(4u);
    CHECK((_as(void*, allocated) >= _as(void*, &alloc) && _as(void*, allocated) < _as(void*, &alloc + 1)));
    alloc.deallocate(allocated, 4u);
}
TEST_CASE("basic | `sys::allocator<int, 16_i16, false>`")
{
    sys::allocator<int, 16_i16, false> alloc;

    CHECK_THROWS_AS(alloc.allocate(32u), std::bad_alloc);

    auto block1 = alloc.allocate(6u);
    auto block2 = alloc.allocate(6u);
    alloc.deallocate(block1, 6u);
    block1 = alloc.allocate(4u);
    alloc.deallocate(block2, 6u);
    alloc.deallocate(block1, 4u);

    byte zeros[16u];
    std::memset(zeros, 0u, sizeof(zeros));
    CHECK(std::memcmp(alloc.bufferUnavail, zeros, sizeof(zeros)) == 0);
}
TEST_CASE("basic | `sys::allocator<int, 0_i16, true>`")
{
    sys::allocator<int, 0_i16, true> alloc;

    alloc.deallocate(alloc.allocate(32u), 32u);
}
