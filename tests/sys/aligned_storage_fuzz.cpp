#include <cstring>

#include <CompilerWarnings.h>
_push_nowarn_deprecated();
_push_nowarn_conv_comp();

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_pop_nowarn_conv_comp();
_pop_nowarn_deprecated();

#include <module/sys>

TEST_CASE("Aligned storage writes match input data. | `sys::aligned_storage`")
{
    rc::check("Fuzz primitives", [](const int64_t& val)
    {
        sys::aligned_storage<int64_t> storage;
        std::memcpy(storage.data(), &val, sizeof(val));
        CHECK(std::memcmp(storage.data(), &val, sizeof(val)) == 0);
    });
}

// NOLINTEND(misc-include-cleaner)
