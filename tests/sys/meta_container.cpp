#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
// Add more module headers as needed...

TEST_CASE("ISizeable<...>", "[sys][ISizeable]")
{
    STATIC_CHECK(sys::ISizeable<float[4]>);
    STATIC_CHECK(sys::ISizeable<std::vector<double>>);
    STATIC_CHECK(sys::ISizeable<std::string>);

    STATIC_CHECK(sys::ISizeable<std::string, void>);
    STATIC_CHECK(sys::ISizeable<std::string, size_t>);
    STATIC_CHECK(sys::ISizeable<std::vector<int>, size_t>);

    STATIC_CHECK_FALSE(sys::ISizeable<int>);
    STATIC_CHECK_FALSE(sys::ISizeable<float>);

    STATIC_CHECK_FALSE(sys::ISizeable<std::vector<int>, int>);
    STATIC_CHECK_FALSE(sys::ISizeable<std::string, ptrdiff_t>);
}

TEST_CASE("IEnumerable<...>", "[sys][IEnumerable]")
{
    STATIC_CHECK(sys::IEnumerable<int[5]>);
    STATIC_CHECK(sys::IEnumerable<char[10]>);
    STATIC_CHECK(sys::IEnumerable<double[3]>);

    STATIC_CHECK(sys::IEnumerable<std::vector<int>>);
    STATIC_CHECK(sys::IEnumerable<std::string>);

    STATIC_CHECK(sys::IEnumerable<std::string, char>);
    STATIC_CHECK(sys::IEnumerable<std::vector<int>, int>);

    STATIC_CHECK_FALSE(sys::IEnumerable<std::vector<int>, float>);
    STATIC_CHECK_FALSE(sys::IEnumerable<std::string, int>);

    STATIC_CHECK_FALSE(sys::IEnumerable<int>);
    STATIC_CHECK_FALSE(sys::IEnumerable<float>);
    STATIC_CHECK_FALSE(sys::IEnumerable<void*>);
}

TEST_CASE("IEmptyQueryable<...>", "[sys][IEmptyQueryable]")
{
    STATIC_CHECK(sys::IEmptyQueryable<double[3]>);
    STATIC_CHECK(sys::IEmptyQueryable<std::string>);

    STATIC_CHECK_FALSE(sys::IEmptyQueryable<void>);
    STATIC_CHECK_FALSE(sys::IEmptyQueryable<double>);
}

TEST_CASE("IAppendable<...>", "[sys][IAppendable]")
{
    STATIC_CHECK(sys::IAppendable<std::vector<double>, double>);
    STATIC_CHECK(sys::IAppendable<std::vector<std::initializer_list<bool>>, bool, bool, bool>);
    STATIC_CHECK(sys::IAppendable<std::string, int>);

    STATIC_CHECK_FALSE(sys::IAppendable<std::string, std::vector<char>>);
    STATIC_CHECK_FALSE(sys::IAppendable<std::type_info, std::string>);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
