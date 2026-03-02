// NOLINTBEGIN(misc-include-cleaner)

#include <CompilerWarnings.h>
_nowarn_begin_conv_comp();
_nowarn_begin_unreachable();

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_nowarn_end_unreachable();
_nowarn_end_conv_comp();

#include <module/sys>
#include <module/sys.Text>

TEST_CASE("Iterating over random bytes never crashes and exhausts buffer.", "[fuzz][sys.Text][codepoint_iter]")
{
    rc::check("Iterating over random bytes never crashes and exhausts buffer.", [](const std::vector<u8::underlying_type>& bytes)
    {
        const char8_t* start = _asr(const char8_t*, bytes.data());
        const char8_t* endPtr = start + bytes.size(); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        sys::codepoint_iter<char8_t> it(start, endPtr);
        const sys::codepoint_iter<char8_t> end(endPtr, endPtr);

        sz count = 0_uz;
        bool doDeref = false;
        while (it != end)
        {
            if (doDeref)
                (void)*it;
            doDeref = !doDeref;
            ++it;
            RC_ASSERT(++count <= sz(bytes.size()));
        }
        RC_ASSERT(it == end);
    });
}

// NOLINTEND(misc-include-cleaner)
