#include <cstdint>

// NOLINTBEGIN(misc-include-cleaner)

#include <CompilerWarnings.h>
_nowarn_begin_deprecated();
_nowarn_begin_conv_comp();
_nowarn_begin_unreachable();

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_nowarn_end_unreachable();
_nowarn_end_conv_comp();
_nowarn_end_deprecated();

#include <module/sys>

TEST_CASE("`i16` => `lsb` + `msb` => `i16` is an invariant.", "[fuzz][sys][xfxx]")
{
    rc::check("`i16` => `lsb` + `msb` => `i16` is an invariant.", [](const int16_t val) { RC_ASSERT(sys::s16fb2(sys::hbfs16(i16(val)), sys::lbfs16(i16(val))) == i16(val)); });
}

// NOLINTEND(misc-include-cleaner)
