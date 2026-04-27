#include <cstdint>
#include <limits>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Sentinels are picked with the right opinion.", "[sys][bsentinel]")
{
    CHECK(sys::bsentinel<int32_t>() == std::numeric_limits<int32_t>::lowest());
    CHECK(sys::bsentinel<uint8_t>() == std::numeric_limits<uint8_t>::max());
    CHECK(sys::bsentinel<int64_t>() == std::numeric_limits<int64_t>::lowest());
    CHECK(sys::bsentinel<uint16_t>() == std::numeric_limits<uint16_t>::max());
    CHECK(sys::bsentinel<int16_t>() == std::numeric_limits<int16_t>::lowest());
    CHECK(sys::bsentinel<char>() == 0);
    CHECK(sys::bsentinel<wchar_t>() == 0);
    CHECK(sys::bsentinel<char16_t>() == 0);
    CHECK(sys::bsentinel<char32_t>() == 0);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
