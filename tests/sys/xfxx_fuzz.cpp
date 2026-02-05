#include <CompilerWarnings.h>
_push_nowarn_deprecated();
_push_nowarn_conv_comp();

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_pop_nowarn_conv_comp();
_pop_nowarn_deprecated();

#include <module/sys>

TEST_CASE("`i16` => `lsb` + `msb` => `i16` is an invariant.", "[fuzz][sys][xfxx]")
{
    rc::check("Validate recombination of high and low bytes.", [](const int16_t val) { CHECK(sys::s16fb2(sys::hbfs16(i16(val)), sys::lbfs16(i16(val))) == i16(val)); });
}

// NOLINTEND(misc-include-cleaner)
