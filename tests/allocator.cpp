#include <cstring>
#include <new>

#define EXPOSE_INTERNALS_FOR_TESTING 1
#include <Allocator.h>
#undef EXPOSE_INTERNALS_FOR_TESTING
#include <CompilerWarnings.h>
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

TEST_CASE("basic | `sys::allocator<int, 16_i16, true>`")
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
TEST_CASE("basic | `sys::allocator<int, 16_i16, false>`")
{
    sys::allocator<int, 16_i16, false> alloc;

    CHECK_THROWS_AS(alloc.allocate(32u), std::bad_alloc);

    auto block1 = alloc.allocate(6u);
    auto block2 = alloc.allocate(6u);
    byte compare[16u] {};
    std::memset(compare, 1u, 12u);
    REQUIRE(std::memcmp(alloc.bufferUnavail, compare, sizeof(compare)) == 0);

    alloc.deallocate(block1, 6u);
    std::memset(compare, 0u, 6u);
    REQUIRE(std::memcmp(alloc.bufferUnavail, compare, sizeof(compare)) == 0);

    block1 = alloc.allocate(4u);
    std::memset(compare, 1u, 4u);
    REQUIRE(std::memcmp(alloc.bufferUnavail, compare, sizeof(compare)) == 0);

    alloc.deallocate(block2, 6u);
    alloc.deallocate(block1, 4u);

    std::memset(compare, 0u, sizeof(compare));
    REQUIRE(std::memcmp(alloc.bufferUnavail, compare, sizeof(compare)) == 0);
}
TEST_CASE("basic | `sys::allocator<int, 0_i16, true>`")
{
    sys::allocator<int, 0_i16, true> alloc;

    try
    {
        //               v Can throw `std::bad_alloc`.
        alloc.deallocate(alloc.allocate(32u), 32u);
    }
    catch (const std::bad_alloc&)
    { }
}
