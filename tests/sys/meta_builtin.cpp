// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("IBuiltinIntegerSigned<...>", "[sys][IBuiltinIntegerSigned]")
{
    STATIC_CHECK(sys::IBuiltinIntegerSigned<signed char>);
    STATIC_CHECK(sys::IBuiltinIntegerSigned<signed short>);
    STATIC_CHECK(sys::IBuiltinIntegerSigned<signed int>);
    STATIC_CHECK(sys::IBuiltinIntegerSigned<signed long>);
    STATIC_CHECK(sys::IBuiltinIntegerSigned<signed long long>);

    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<unsigned char>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<unsigned short>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<unsigned int>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<unsigned long>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<unsigned long long>);

    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<float>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<double>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<unsigned char>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<bool>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<void>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerSigned<std::string>);
}

TEST_CASE("IBuiltinIntegerUnsigned<...>", "[sys][IBuiltinIntegerUnsigned]")
{
    STATIC_CHECK(sys::IBuiltinIntegerUnsigned<unsigned char>);
    STATIC_CHECK(sys::IBuiltinIntegerUnsigned<unsigned short>);
    STATIC_CHECK(sys::IBuiltinIntegerUnsigned<unsigned int>);
    STATIC_CHECK(sys::IBuiltinIntegerUnsigned<unsigned long>);
    STATIC_CHECK(sys::IBuiltinIntegerUnsigned<unsigned long long>);

    STATIC_CHECK_FALSE(sys::IBuiltinIntegerUnsigned<signed char>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerUnsigned<signed short>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerUnsigned<signed int>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerUnsigned<signed long>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerUnsigned<signed long long>);

    STATIC_CHECK_FALSE(sys::IBuiltinIntegerUnsigned<float>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerUnsigned<double>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerUnsigned<bool>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerUnsigned<std::string>);
}

TEST_CASE("IBuiltinInteger<...>", "[sys][IBuiltinInteger]")
{
    STATIC_CHECK(sys::IBuiltinInteger<char>);
    STATIC_CHECK(sys::IBuiltinInteger<signed char>);
    STATIC_CHECK(sys::IBuiltinInteger<unsigned char>);
    STATIC_CHECK(sys::IBuiltinInteger<signed int>);
    STATIC_CHECK(sys::IBuiltinInteger<unsigned int>);
    STATIC_CHECK(sys::IBuiltinInteger<signed long long /* NOLINT(google-runtime-int) */>);
    STATIC_CHECK(sys::IBuiltinInteger<unsigned long long /* NOLINT(google-runtime-int) */>);

    STATIC_CHECK_FALSE(sys::IBuiltinInteger<float>);
    STATIC_CHECK_FALSE(sys::IBuiltinInteger<double>);
    STATIC_CHECK_FALSE(sys::IBuiltinInteger<long double /* NOLINT(google-runtime-float) */>);

    STATIC_CHECK_FALSE(sys::IBuiltinInteger<bool>);
    STATIC_CHECK_FALSE(sys::IBuiltinInteger<std::string>);
}

TEST_CASE("IBuiltinFloatingPoint<...>", "[sys][IBuiltinFloatingPoint]")
{
    STATIC_CHECK(sys::IBuiltinFloatingPoint<float>);
    STATIC_CHECK(sys::IBuiltinFloatingPoint<double>);
    STATIC_CHECK(sys::IBuiltinFloatingPoint<long double /* NOLINT(google-runtime-float) */>);

    STATIC_CHECK_FALSE(sys::IBuiltinFloatingPoint<int>);
    STATIC_CHECK_FALSE(sys::IBuiltinFloatingPoint<unsigned int>);
    STATIC_CHECK_FALSE(sys::IBuiltinFloatingPoint<long long /* NOLINT(google-runtime-int) */>);

    STATIC_CHECK_FALSE(sys::IBuiltinFloatingPoint<bool>);
    STATIC_CHECK_FALSE(sys::IBuiltinFloatingPoint<char>);
    STATIC_CHECK_FALSE(sys::IBuiltinFloatingPoint<std::string>);
}

TEST_CASE("IBuiltinNumeric<...>", "[sys][IBuiltinNumeric]")
{
    STATIC_CHECK(sys::IBuiltinNumeric<char>);
    STATIC_CHECK(sys::IBuiltinNumeric<signed int>);
    STATIC_CHECK(sys::IBuiltinNumeric<unsigned long /* NOLINT(google-runtime-int) */>);
    STATIC_CHECK(sys::IBuiltinNumeric<float>);
    STATIC_CHECK(sys::IBuiltinNumeric<double>);
    STATIC_CHECK(sys::IBuiltinNumeric<long double /* NOLINT(google-runtime-float) */>);

    STATIC_CHECK_FALSE(sys::IBuiltinNumeric<bool>);
    STATIC_CHECK_FALSE(sys::IBuiltinNumeric<std::string>);
    STATIC_CHECK_FALSE(sys::IBuiltinNumeric<void*>);
}

TEST_CASE("IBuiltinIntegerNarrowerThan<...>", "[sys][IBuiltinIntegerNarrowerThan]")
{
    STATIC_CHECK(sys::IBuiltinIntegerNarrowerThan<signed char, signed long long /* NOLINT(google-runtime-int) */>);
    STATIC_CHECK(sys::IBuiltinIntegerNarrowerThan<signed short, signed long long /* NOLINT(google-runtime-int) */>);
    STATIC_CHECK(sys::IBuiltinIntegerNarrowerThan<signed int, signed long long /* NOLINT(google-runtime-int) */>);

    STATIC_CHECK(sys::IBuiltinIntegerNarrowerThan<unsigned char, unsigned long long /* NOLINT(google-runtime-int) */>);
    STATIC_CHECK(sys::IBuiltinIntegerNarrowerThan<unsigned short, unsigned long long /* NOLINT(google-runtime-int) */>);
    STATIC_CHECK(sys::IBuiltinIntegerNarrowerThan<unsigned int, unsigned long long /* NOLINT(google-runtime-int) */>);

    STATIC_CHECK(sys::IBuiltinIntegerNarrowerThan<signed int, signed int>);
    STATIC_CHECK(sys::IBuiltinIntegerNarrowerThan<unsigned int, unsigned int>);

    STATIC_CHECK_FALSE(sys::IBuiltinIntegerNarrowerThan<signed int, signed char>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerNarrowerThan<unsigned int, unsigned char>);

    STATIC_CHECK_FALSE(sys::IBuiltinIntegerNarrowerThan<signed int, unsigned int>);
    STATIC_CHECK_FALSE(sys::IBuiltinIntegerNarrowerThan<signed char, unsigned long long /* NOLINT(google-runtime-int) */>);

    STATIC_CHECK_FALSE(sys::IBuiltinIntegerNarrowerThan<unsigned int, signed int>);
}

TEST_CASE("ICharacter<...>", "[sys][ICharacter]")
{
    STATIC_CHECK(sys::ICharacter<char>);
    STATIC_CHECK(sys::ICharacter<wchar_t>);
    STATIC_CHECK(sys::ICharacter<char8_t>);
    STATIC_CHECK(sys::ICharacter<char16_t>);
    STATIC_CHECK(sys::ICharacter<char32_t>);

    STATIC_CHECK_FALSE(sys::ICharacter<int>);
    STATIC_CHECK_FALSE(sys::ICharacter<signed char>);
    STATIC_CHECK_FALSE(sys::ICharacter<unsigned char>);
    STATIC_CHECK_FALSE(sys::ICharacter<std::string>);
    STATIC_CHECK_FALSE(sys::ICharacter<bool>);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
