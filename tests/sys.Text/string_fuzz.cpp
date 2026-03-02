#include <vector>

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

TEST_CASE("Join of split by same delimiter is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("Join of split by same delimiter is invariant.", [](const std::vector<u8::underlying_type>& bytes)
    {
        sys::str s;
        for (const u8::underlying_type v : bytes)
            if (v)
                s.append(_as(char8_t, v));

        RC_ASSERT(sys::str::join(s.split(u8','), u8",") == s);
    });
}

TEST_CASE("UTF-8 -> UTF-32 -> UTF-8 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-8 -> UTF-32 -> UTF-8 is invariant.", [](const std::string& input)
    {
        sys::str s(std::span<const char8_t>(_asr(const char8_t*, input.data()) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str, _as(sys::str32, s)));
    });
}
TEST_CASE("UTF-8 -> UTF-16 -> UTF-8 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-8 -> UTF-16 -> UTF-8 is invariant.", [](const std::string& input)
    {
        sys::str s(std::span<const char8_t>(_asr(const char8_t*, input.data()) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str, _as(sys::str16, s)));
    });
}

TEST_CASE("UTF-16 -> UTF-32 -> UTF-16 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-16 -> UTF-32 -> UTF-16 is invariant.", [](const std::vector<u16::underlying_type>& input)
    {
        sys::str16 s(std::span<const char16_t>(_asr(const char16_t*, input.data()) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str16, _as(sys::str32, s)));
    });
}
TEST_CASE("UTF-16 -> UTF-8 -> UTF-16 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-16 -> UTF-8 -> UTF-16 is invariant.", [](const std::vector<u16::underlying_type>& input)
    {
        sys::str16 s(std::span<const char16_t>(_asr(const char16_t*, input.data()) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str16, _as(sys::str, s)));
    });
}

TEST_CASE("UTF-32 -> UTF-16 -> UTF-32 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-32 -> UTF-16 -> UTF-32 is invariant.", [](const std::vector<u32::underlying_type>& input)
    {
        sys::str32 s(std::span<const char32_t>(_asr(const char32_t*, input.data()) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str32, _as(sys::str16, s)));
    });
}
TEST_CASE("UTF-32 -> UTF-8 -> UTF-32 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-32 -> UTF-8 -> UTF-32 is invariant.", [](const std::vector<u32::underlying_type>& input)
    {
        sys::str32 s(std::span<const char32_t>(_asr(const char32_t*, input.data()) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str32, _as(sys::str, s)));
    });
}

TEST_CASE("Trim reduces size or stays same.", "[fuzz][sys.Text][string]")
{
    rc::check("Trim reduces size or stays same.", [](const std::string& input)
    {
        const sys::str s(std::span<const char8_t>(_asr(const char8_t*, input.data()) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        const sys::str trimmed = sys::str(s).trim();
        RC_ASSERT(trimmed.size() <= s.size());
        RC_ASSERT(s.contains(trimmed));

        RC_ASSERT(trimmed == sys::str(trimmed).trim());
    });
}

// NOLINTEND(misc-include-cleaner)
