#include <array>
#include <cstdint>
#include <cstring>

#include <CompilerWarnings.h>
_nowarn_begin_deprecated();
_nowarn_begin_conv_comp();
_nowarn_begin_unreachable();

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_nowarn_end_unreachable();
_nowarn_end_conv_comp();
_nowarn_end_deprecated();

#include <module/sys>

TEST_CASE("Aligned storage writes match input data.", "[fuzz][sys][aligned_storage]")
{
    rc::check("Aligned storage writes match input data.", [](const std::array<int64_t, 1024uz>& val /* NOLINT(readability-magic-numbers) */)
    {
        sys::aligned_storage<std::array<int64_t, 1024uz /* NOLINT(readability-magic-numbers) */>> storage;
        std::memcpy(storage.data(), &val, sizeof(val));
        RC_ASSERT(std::memcmp(storage.data(), &val, sizeof(val)) == 0);
    });
}

// NOLINTEND(misc-include-cleaner)
