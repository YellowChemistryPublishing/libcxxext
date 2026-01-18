// NOLINTBEGIN(misc-include-cleaner)

#include <CompilerWarnings.h>
_push_nowarn_deprecated();
_push_nowarn_conv_comp();

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_pop_nowarn_conv_comp();
_pop_nowarn_deprecated();

#include <cxxsup.h>

TEST_CASE("`i16` => lsb + msb => `i16` is an invariant. | `sys::s16fb2(...)`, `sys::hbfs16(...)`, `sys::lbfs16(...)`")
{
    rc::check("Validate recombination of high and low bytes.", [](const int16_t val) { CHECK(sys::s16fb2(sys::hbfs16(val), sys::lbfs16(val)) == val); });
}

// NOLINTEND(misc-include-cleaner)
