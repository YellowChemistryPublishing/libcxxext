#include <cstring>
#include <new>

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <Allocator.h>
#include <CompilerWarnings.h>

TEST_CASE("Allocation and deallocation works as expected. | `sys::allocator<int, 16_i16, true>`")
{
    sys::allocator<int, 16_i16, true> alloc;

    int* allocated = nullptr;
    REQUIRE_NOTHROW(allocated = alloc.allocate(24u));

    CHECK((_as(void*, allocated) < _as(void*, &alloc) || _as(void*, allocated) > _as(void*, &alloc + 1)));
    alloc.deallocate(allocated, 24u);

    allocated = alloc.allocate(4u);
    CHECK((_as(void*, allocated) >= _as(void*, &alloc) && _as(void*, allocated) < _as(void*, &alloc + 1)));
    alloc.deallocate(allocated, 4u);
}
TEST_CASE("Static buffer allocator works as expected. | `sys::allocator<int, 16_i16, false>`")
{
    sys::allocator<int, 16_i16, false> alloc;

    CHECK_THROWS_AS(alloc.allocate(32u), std::bad_alloc);

    auto block1 = alloc.allocate(6u);
    auto block2 = alloc.allocate(6u);
    byte compare[16u] {};
    std::memset(compare, 1u, 12u);

    alloc.deallocate(block1, 6u);
    std::memset(compare, 0u, 6u);

    block1 = alloc.allocate(4u);
    std::memset(compare, 1u, 4u);

    alloc.deallocate(block2, 6u);
    alloc.deallocate(block1, 4u);

    std::memset(compare, 0u, sizeof(compare));
}
TEST_CASE("Dynamic buffer allocator works as expected. | `sys::allocator<int, 0_i16, true>`")
{
    sys::allocator<int, 0_i16, true> alloc;
    alloc.deallocate(alloc.allocate(32u), 32u);
}
