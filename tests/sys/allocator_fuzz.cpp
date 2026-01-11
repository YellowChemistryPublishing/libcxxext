#include <CompilerWarnings.h>
_push_nowarn_clang(_clWarn_clang_deprecated);
_push_nowarn_conv_comp();

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <rapidcheck.h>

_pop_nowarn_conv_comp();
_pop_nowarn_clang();

#include <Allocator.h>

TEST_CASE("Can allocate and write to buffer. | `sys::allocator<unsigned char, 4_i16, true>`")
{
    rc::check("Fuzz", [](const std::vector<uint16_t>& data)
    {
        sys::allocator<unsigned char, 4_i16, true> alloc;
        std::vector<unsigned char*> toFree;
        for (uint16_t bufSize : data)
        {
            unsigned char* ptr = alloc.allocate(bufSize);
            for (uint16_t i = 0; i < bufSize; i++) *_asr(volatile unsigned char*, &ptr[i]) = _as(unsigned char, i);
            toFree.push_back(ptr);
        }

        for (unsigned char* ptr : toFree) alloc.deallocate(ptr, 0);
    });
}
