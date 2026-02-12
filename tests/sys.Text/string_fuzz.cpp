#include <CompilerWarnings.h>
_push_nowarn_conv_comp();

#include <cstdint>
#include <vector>

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>

#include <module/sys>
#include <module/sys.Text>

_pop_nowarn_conv_comp();

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("`sys::str<...>` Fuzzing", "[fuzz][sys.Text][string]")
{
    rc::check("Join of split by same delimiter is invariant.", [](const std::vector<uint8_t>& bytes)
    {
        sys::str s;
        for (const uint8_t v : bytes)
            if (v)
                s.append(_as(char8_t, v));

        const std::vector<sys::str> parts = s.split(u8',');
        const sys::str joined = sys::str::join(parts, u8",");
        RC_ASSERT(joined == s);
    });

    rc::check("UTF-8 -> UTF-32 -> UTF-8 is invariant.", [](const std::string& input)
    {
        sys::str s(std::span<const char8_t>(_asr(const char8_t*, input.data()), input.size())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str, _as(sys::str32, s)));
    });
    rc::check("UTF-8 -> UTF-16 -> UTF-8 is invariant.", [](const std::string& input)
    {
        sys::str s(std::span<const char8_t>(_asr(const char8_t*, input.data()), input.size())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str, _as(sys::str16, s)));
    });
    rc::check("UTF-16 -> UTF-32 -> UTF-16 is invariant.", [](const std::vector<uint16_t>& input)
    {
        sys::str16 s(std::span<const char16_t>(_asr(const char16_t*, input.data()), input.size())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str16, _as(sys::str32, s)));
    });
    rc::check("UTF-16 -> UTF-8 -> UTF-16 is invariant.", [](const std::vector<uint16_t>& input)
    {
        sys::str16 s(std::span<const char16_t>(_asr(const char16_t*, input.data()), input.size())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str16, _as(sys::str, s)));
    });
    rc::check("UTF-32 -> UTF-16 -> UTF-32 is invariant.", [](const std::vector<uint32_t>& input)
    {
        sys::str32 s(std::span<const char32_t>(_asr(const char32_t*, input.data()), input.size())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str32, _as(sys::str16, s)));
    });
    rc::check("UTF-32 -> UTF-8 -> UTF-32 is invariant.", [](const std::vector<uint32_t>& input)
    {
        sys::str32 s(std::span<const char32_t>(_asr(const char32_t*, input.data()), input.size())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        s.replace_invalid();
        RC_ASSERT(s == _as(sys::str32, _as(sys::str, s)));
    });

    rc::check("Trim reduces size or stays same.", [](const std::string& input)
    {
        const sys::str s(std::span<const char8_t>(_asr(const char8_t*, input.data()), input.size())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        const sys::str trimmed = sys::str(s).trim();
        RC_ASSERT(trimmed.size() <= s.size());
        RC_ASSERT(s.contains(trimmed));

        const sys::str doubleTrimmed = sys::str(trimmed).trim();
        RC_ASSERT(trimmed == doubleTrimmed);
    });
}

// NOLINTEND(misc-include-cleaner)
