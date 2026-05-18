#include <cstdint>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
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

TEST_CASE("[[fuzz]] x_msb = hbfs16(x), x_lsb = lbfs16(x) => s16fb2(x_msb, x_lsb) == x", "[fuzz][sys][s16fb2][hbfs16][lbfs16]")
{
    rc::check("`i16` => `lsb` + `msb` => `i16` is an invariant.",
              [](const int16_t val) -> void { RC_ASSERT(sys::s16fb2(sys::hbfs16(i16(val)), sys::lbfs16(i16(val))) == i16(val)); });
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
