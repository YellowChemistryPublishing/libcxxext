// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("meta::type_switch<...>, meta::type_switch_cases<...>, meta::type_case<...>", "[sys][meta][type_switch][type_switch_cases][type_case]")
{
    using sys::meta::type_switch, sys::meta::type_switch_cases, sys::meta::type_case;

    STATIC_CHECK(type_switch_cases<type_case<std::same_as<int, int>, int>, type_case<std::same_as<int, int>, bool>>::count_returns() == 2uz);
    STATIC_CHECK(type_switch_cases<type_case<std::same_as<int, bool>, int>, type_case<std::same_as<bool, int>, bool>>::count_returns() == 0uz);

    STATIC_CHECK(std::same_as<type_switch<type_case<std::same_as<size_t, wchar_t>, long double /* NOLINT(google-runtime-float) */>, type_case<std::same_as<int, int>, int>,
                                          type_case<std::same_as<int, float>, float>>,
                              int>);
    STATIC_CHECK(std::same_as<type_switch<type_case<std::same_as<float, double>, float>, type_case<false, bool>, type_case<true, void>>, void>);
}

TEST_CASE("meta::replace_cv<...>", "[sys][meta][replace_cv]")
{
    STATIC_CHECK(std::same_as<sys::meta::replace_cv<int, const float>, const int>);
    STATIC_CHECK(std::same_as<sys::meta::replace_cv<int, volatile float>, volatile int>);
    STATIC_CHECK(std::same_as<sys::meta::replace_cv<int, const volatile float>, const volatile int>);

    STATIC_CHECK(std::same_as<sys::meta::replace_cv<int, const float&>, const int>);
    STATIC_CHECK(std::same_as<sys::meta::replace_cv<int, volatile float&>, volatile int>);
    STATIC_CHECK(std::same_as<sys::meta::replace_cv<int, const volatile float&>, const volatile int>);
    STATIC_CHECK(std::same_as<sys::meta::replace_cv<int, const float&&>, const int>);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
