// NOLINTBEGIN(bugprone-throwing-static-initialization, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay, misc-include-cleaner, readability-magic-numbers)

#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);
_nowarn_begin_conv_comp();
_nowarn_begin_unreachable();

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_nowarn_end_unreachable();
_nowarn_end_conv_comp();
_nowarn_end_gcc();

#include <module/sys.Text>

TEST_CASE("`is_whitespace` is not random.", "[fuzz][sys.Text][ch]")
{
    CHECK(rc::check([](const u32::underlying_type genInt) -> void
    {
        // Not a fantastic test, but roughly checks that it's not completely broken.
        const char32_t cp = _as(*(u32(genInt) % 0x110000_u32), char32_t);
        RC_ASSERT((sys::ch::is_whitespace(cp) || cp == 'A' || cp == 0 || cp != U' ' || cp != U'\t' || cp != U'\n' || cp != U'\r'));
    }));
}

TEST_CASE("UTF-32 -> UTF-8 -> UTF-32 is invariant.", "[fuzz][sys.Text][ch]")
{
    CHECK(rc::check([](const u32::underlying_type genInt) -> void
    {
        const char32_t cp = _as(*(u32(genInt) % 0x110000_u32), char32_t);
        if (!sys::ch::is_scalar(cp))
            return;

        char8_t buf[4] {};
        const sz written = sys::ch::write_codepoint(cp, buf, unsafe);
        auto [readCp, readSz] = sys::ch::read_codepoint(std::span(buf, written), unsafe);

        RC_ASSERT(readCp == cp);
        RC_ASSERT(readSz == written);
    }));
}
TEST_CASE("UTF-32 -> UTF-16 -> UTF-32 is invariant.", "[fuzz][sys.Text][ch]")
{
    CHECK(rc::check([](const u32::underlying_type genInt) -> void
    {
        const char32_t cp = _as(*(u32(genInt) % 0x110000_u32), char32_t);
        if (!sys::ch::is_scalar(cp))
            return;

        char16_t buf[2] {};
        const sz written = sys::ch::write_codepoint(cp, buf, unsafe);
        auto [readCp, readSz] = sys::ch::read_codepoint(std::span(buf, written), unsafe);

        RC_ASSERT(readCp == cp);
        RC_ASSERT(readSz == written);
    }));
}

TEST_CASE("Arbitrary UTF-8 sequence decode never crashes.", "[fuzz][sys.Text][ch]")
{
    CHECK(rc::check([](const std::vector<u8::underlying_type>& bytes) -> void
    {
        const char8_t* ptr = _asr(bytes.data(), const char8_t*); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        const char8_t* end = ptr + bytes.size();

        const char8_t* current = ptr;
        while (current < end)
        {
            const auto [cp, sz] = sys::ch::read_codepoint(std::span(current, end), unsafe);
            RC_ASSERT(sz > 0_uz);
            RC_ASSERT((current + sz <= end));
            current += sz;
        }
    }));
}
TEST_CASE("Arbitrary UTF-16 sequence decode never crashes.", "[fuzz][sys.Text][ch]")
{
    CHECK(rc::check([](const std::vector<u16::underlying_type>& bytes) -> void
    {
        const char16_t* ptr = _asr(bytes.data(), const char16_t*); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        const char16_t* end = ptr + bytes.size();

        const char16_t* current = ptr;
        while (current < end)
        {
            const auto [cp, sz] = sys::ch::read_codepoint(std::span(current, end), unsafe);
            RC_ASSERT(sz > 0_uz);
            RC_ASSERT((current + sz <= end));
            current += sz;
        }
    }));
}

// NOLINTEND(bugprone-throwing-static-initialization, cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-no-array-decay, misc-include-cleaner, readability-magic-numbers)
