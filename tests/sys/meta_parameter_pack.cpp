// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

TEST_CASE("meta::parameter_pack<...>", "[sys][meta][parameter_pack]")
{
    using example_meta = sys::meta::parameter_pack<int, float, double, char>;

    STATIC_CHECK(std::same_as<example_meta::tuple, std::tuple<int, float, double, char>>);

    STATIC_CHECK(std::same_as<example_meta::at<0>, int>);
    STATIC_CHECK(std::same_as<example_meta::at<1>, float>);
    STATIC_CHECK(std::same_as<example_meta::at<2>, double>);
    STATIC_CHECK(std::same_as<example_meta::at<3>, char>);

    STATIC_CHECK(example_meta::contains<int>());
    STATIC_CHECK(example_meta::contains<float>());
    STATIC_CHECK(example_meta::contains<double>());
    STATIC_CHECK(example_meta::contains<char>());

    STATIC_CHECK_FALSE(example_meta::contains<long /* NOLINT(google-runtime-int) */>());
    STATIC_CHECK_FALSE(example_meta::contains<std::string>());
    STATIC_CHECK_FALSE(example_meta::contains<void>());

    using example_empty_meta = sys::meta::parameter_pack<>;
    STATIC_CHECK(std::same_as<example_empty_meta::tuple, std::tuple<>>);
    STATIC_CHECK_FALSE(example_empty_meta::contains<int>());

    using example_single_meta = sys::meta::parameter_pack<int>;
    STATIC_CHECK(std::same_as<example_single_meta::at<0>, int>);
    STATIC_CHECK(example_single_meta::contains<int>());
    STATIC_CHECK_FALSE(example_single_meta::contains<float>());

    using example_multi_meta = sys::meta::parameter_pack<int, int, int>;
    STATIC_CHECK(example_multi_meta::contains<int>());
    STATIC_CHECK(std::same_as<example_multi_meta::at<0>, int>);
    STATIC_CHECK(std::same_as<example_multi_meta::at<2>, int>);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
