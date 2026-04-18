#include <array>
#include <cstdint>
#include <cstring>

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

TEST_CASE("Aligned storage writes match input data.", "[fuzz][sys][aligned_storage]")
{
    rc::check("Aligned storage writes match input data.", [](const std::array<int64_t, 1024uz>& val /* NOLINT(readability-magic-numbers) */) -> void
    {
        sys::aligned_storage<std::array<int64_t, 1024uz /* NOLINT(readability-magic-numbers) */>> storage;
        std::memcpy(storage.data(), &val, sizeof(val));
        RC_ASSERT(std::memcmp(storage.data(), &val, sizeof(val)) == 0);
    });
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
