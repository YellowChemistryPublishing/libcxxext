#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

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

TEST_CASE("[[fuzz]] sys::aligned_storage<...>", "[fuzz][sys][aligned_storage]")
{
    CHECK(rc::check([](const std::array<int64_t, 1024uz /* NOLINT(readability-magic-numbers) */>& val) -> void
    {
        sys::aligned_storage<std::array<int64_t, 1024uz /* NOLINT(readability-magic-numbers) */>, std::array<uint64_t, 2048uz /* NOLINT(readability-magic-numbers) */>,
                             std::max_align_t>
            storage;

        RC_ASSERT(_asr(storage.data(), uintptr_t) % alignof(std::max_align_t) == 0uz);
        RC_ASSERT(sizeof(storage) == sizeof(std::array<uint64_t, 2048uz /* NOLINT(readability-magic-numbers) */>));

        std::construct_at(storage.data(), val);
        RC_ASSERT(*storage.data() == val);

        std::destroy_at(storage.data());
    }));
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
