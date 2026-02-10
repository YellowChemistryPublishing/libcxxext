#include <CompilerWarnings.h>
_push_nowarn_conv_comp();

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

#include <module/sys>
#include <module/sys.Text>

_pop_nowarn_conv_comp();

TEST_CASE("`sys::str32_iter<...>` Fuzzing", "[fuzz][sys.Text][str32_iter]")
{
    rc::check("Iterating over random bytes never crashes and exhausts buffer.", [](const std::vector<uint8_t>& bytes)
    {
        const char8_t* start = _asr(const char8_t*, bytes.data());
        const char8_t* endPtr = start + bytes.size(); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        sys::str32_iter<char8_t> it(start, endPtr);
        const sys::str32_iter<char8_t> end(endPtr, endPtr);

        sz count = 0_uz;
        bool doDeref = false;
        while (it != end)
        {
            if (doDeref)
                (void)*it;
            doDeref = !doDeref;
            ++it;
            CHECK(++count <= sz(bytes.size()));
        }
        CHECK(it == end);
    });
}

// NOLINTEND(misc-include-cleaner)
