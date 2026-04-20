#include <vector>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)

#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);
_nowarn_begin_conv_comp();
_nowarn_begin_unreachable();

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

_nowarn_end_unreachable();
_nowarn_end_conv_comp();
_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Text>

TEST_CASE("Join of split by same delimiter is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("Join of split by same delimiter is invariant.", [](const std::vector<u8::underlying_type>& bytes) -> void
    {
        sys::str s;
        for (const u8::underlying_type v : bytes)
            if (v)
                s.append(_as(v, char8_t));

        RC_ASSERT(sys::str::join(s.split(u8','), u8",") == s);
    });
}

TEST_CASE("UTF-8 -> UTF-32 -> UTF-8 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-8 -> UTF-32 -> UTF-8 is invariant.", [](const std::string& input) -> void
    {
        sys::str s(std::span<const char8_t>(_asr(input.data(), const char8_t*) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(_as(s, sys::str32), sys::str));
    });
}
TEST_CASE("UTF-8 -> UTF-16 -> UTF-8 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-8 -> UTF-16 -> UTF-8 is invariant.", [](const std::string& input) -> void
    {
        sys::str s(std::span<const char8_t>(_asr(input.data(), const char8_t*) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(_as(s, sys::str16), sys::str));
    });
}

TEST_CASE("UTF-16 -> UTF-32 -> UTF-16 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-16 -> UTF-32 -> UTF-16 is invariant.", [](const std::vector<u16::underlying_type>& input) -> void
    {
        sys::str16 s(std::span<const char16_t>(_asr(input.data(), const char16_t*) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(_as(s, sys::str32), sys::str16));
    });
}
TEST_CASE("UTF-16 -> UTF-8 -> UTF-16 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-16 -> UTF-8 -> UTF-16 is invariant.", [](const std::vector<u16::underlying_type>& input) -> void
    {
        sys::str16 s(std::span<const char16_t>(_asr(input.data(), const char16_t*) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(_as(s, sys::str), sys::str16));
    });
}

TEST_CASE("UTF-32 -> UTF-16 -> UTF-32 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-32 -> UTF-16 -> UTF-32 is invariant.", [](const std::vector<u32::underlying_type>& input) -> void
    {
        sys::str32 s(std::span<const char32_t>(_asr(input.data(), const char32_t*) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(_as(s, sys::str16), sys::str32));
    });
}
TEST_CASE("UTF-32 -> UTF-8 -> UTF-32 is invariant.", "[fuzz][sys.Text][string]")
{
    rc::check("UTF-32 -> UTF-8 -> UTF-32 is invariant.", [](const std::vector<u32::underlying_type>& input) -> void
    {
        sys::str32 s(std::span<const char32_t>(_asr(input.data(), const char32_t*) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        s.replace_invalid();
        RC_ASSERT(s == _as(_as(s, sys::str), sys::str32));
    });
}

TEST_CASE("Trim reduces size or stays same.", "[fuzz][sys.Text][string]")
{
    rc::check("Trim reduces size or stays same.", [](const std::string& input) -> void
    {
        const sys::str s(std::span<const char8_t>(_asr(input.data(), const char8_t*) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */, input.size()));
        const sys::str trimmed = sys::str(s).trim();
        RC_ASSERT(trimmed.size() <= s.size());
        RC_ASSERT(s.contains(trimmed));

        RC_ASSERT(trimmed == sys::str(trimmed).trim());
    });
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
