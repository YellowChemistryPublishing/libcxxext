// NOLINTBEGIN(misc-include-cleaner, readability-magic-numbers, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay)

#include <CompilerWarnings.h>
_push_nowarn_conv_comp();

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_pop_nowarn_conv_comp();

#include <module/sys.Text>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::ch` Fuzzing", "[fuzz][sys.Text][char]")
{
    rc::check("`is_whitespace` is not random.", [](const uint_least32_t genInt)
    {
        // Not a fantastic test, but roughly checks that it's not completely broken.
        const char32_t cp = _as(char32_t, u32(genInt) % 0x110000_u32);
        CHECK((sys::ch::is_whitespace(cp) || cp == 'A' || cp == 0 || cp != U' ' || cp != U'\t' || cp != U'\n' || cp != U'\r'));
    });

    rc::check("UTF-32 -> UTF-8 -> UTF-32 is invariant.", [](const uint_least32_t genInt)
    {
        const char32_t cp = _as(char32_t, u32(genInt) % 0x110000_u32);
        char8_t buf[4] {};
        const sz written = sys::ch::write_codepoint(cp, buf, unsafe());
        auto [readCp, readSz] = sys::ch::read_codepoint(std::span(buf, written), unsafe());

        CHECK(readCp == cp);
        CHECK(readSz == written);
    });
    rc::check("UTF-32 -> UTF-16 -> UTF-32 is invariant.", [](const uint_least32_t genInt)
    {
        const char32_t cp = _as(char32_t, u32(genInt) % 0x110000_u32);
        char16_t buf[2] {};
        const sz written = sys::ch::write_codepoint(cp, buf, unsafe());
        auto [readCp, readSz] = sys::ch::read_codepoint(std::span(buf, written), unsafe());

        CHECK(readCp == cp);
        CHECK(readSz == written);
    });

    rc::check("Arbitary UTF-8 sequence decode never crashes.", [](const std::vector<uint_least8_t>& bytes)
    {
        const char8_t* ptr = _asr(const char8_t*, bytes.data()); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        const char8_t* end = ptr + bytes.size();

        const char8_t* current = ptr;
        while (current < end)
        {
            const auto [cp, sz] = sys::ch::read_codepoint(std::span(current, end), unsafe());
            REQUIRE(sz > 0_uz);
            CHECK(current + sz <= end);
            current += sz;
        }
    });
    rc::check("Arbitary UTF-16 sequence decode never crashes.", [](const std::vector<uint_least16_t>& bytes)
    {
        const char16_t* ptr = _asr(const char16_t*, bytes.data()); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        const char16_t* end = ptr + bytes.size();

        const char16_t* current = ptr;
        while (current < end)
        {
            const auto [cp, sz] = sys::ch::read_codepoint(std::span(current, end), unsafe());
            REQUIRE(sz > 0_uz);
            CHECK(current + sz <= end);
            current += sz;
        }
    });
}

// NOLINTEND(misc-include-cleaner, readability-magic-numbers, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay)
