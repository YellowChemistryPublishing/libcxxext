#include <memory>
#include <string>
#include <string_view>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_clang(_clwarn_clang_consumed);
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

using namespace std::string_view_literals;

TEST_CASE("_res_movret(...)", "[sys][result][macros]")
{
    const auto runTest = [](auto&& v) -> i32
    {
        _res_movret(const i16 val, (sys::result<i16, i32>(v)));
        return { i32(val) + 1_i32 };
    };

    CHECK(runTest(10_i16) == 11_i32);
    CHECK(runTest(404_i32) == 404_i32);
}

TEST_CASE("internal::result_b_err<...>::ctor_err(...), err(...)", "[sys][result]")
{
    {
        sys::result<void, i32> res = 123_i32 /* NOLINT(readability-magic-numbers) */;
        const i32 got = res.expect_err();
        CHECK(got == 123_i32);
    }

    {
        i32 err = i32::ones();
        sys::result<void, i32&> res = err;
        const i32& got = res.expect_err();

        CHECK(std::addressof(got) == std::addressof(err));
        CHECK(got == i32::ones());
    }

    {
        sys::result<void, std::string> res(12uz /* NOLINT(readability-magic-numbers) */, 'a');
        const std::string got = res.expect_err();
        CHECK(got == "aaaaaaaaaaaa");
    }
}

TEST_CASE("internal::result_b_err<...>::operator Result<T, void>()", "[sys][result]")
{
    {
        sys::result<void, i32> res;
        const sys::result<void> got = _as(std::move(res), sys::result<void>);
        CHECK(got);
    }

    {
        sys::result<i32, i32> res = 123_i32 /* NOLINT(readability-magic-numbers) */; // Should resolve to good result.
        const i32 got = _as(std::move(res), sys::result<i32>).expect();
        CHECK(got == 123_i32);
    }

    {
        sys::result<i64, i32> res = 123_i32 /* NOLINT(readability-magic-numbers) */;
        const sys::result<i64> got = _as(std::move(res), sys::result<i64>) /* NOLINT(clang-analyzer-cplusplus.Move, bugprone-use-after-move) */;
        CHECK(got.operator!());
    }

    {
        sys::result<void, i32> res = 123_i32 /* NOLINT(readability-magic-numbers) */;
        (void)sys::result<void, i32>(std::move(res));
        const sys::result<void> got = _as(std::move(res), sys::result<void>) /* NOLINT(clang-analyzer-cplusplus.Move, bugprone-use-after-move) */;
        CHECK_FALSE(got.operator bool());
        CHECK_FALSE(got.operator!());
    }

    {
        sys::result<i64, i32> res = 123_i32 /* NOLINT(readability-magic-numbers) */;
        (void)sys::result<i64, i32>(std::move(res));
        const sys::result<i64> got = _as(std::move(res), sys::result<i64>) /* NOLINT(clang-analyzer-cplusplus.Move, bugprone-use-after-move) */;
        CHECK_FALSE(got.operator bool());
        CHECK_FALSE(got.operator!());
    }
}

TEST_CASE("internal::result_b_ok<...>::ctor_ok(...), move(...)", "[sys][result]")
{
    {
        sys::result<i32, void> res = 123_i32 /* NOLINT(readability-magic-numbers) */;
        const i32 got = res.expect();
        CHECK(got == 123_i32);
    }

    {
        i32 err = i32::ones();
        sys::result<i32&, void> res = err;
        const i32& got = res.expect();

        CHECK(std::addressof(got) == std::addressof(err));
        CHECK(got == i32::ones());
    }

    {
        sys::result<std::string, void> res(4uz /* NOLINT(readability-magic-numbers) */, 'b');
        const std::string got = res.expect();
        CHECK(got == "bbbb");
    }
}

TEST_CASE("result<T, U>::result(...)", "[sys][result]")
{
    CHECK(sys::result<i32, const char*>(123_i32 /* NOLINT(readability-magic-numbers) */).expect() == 123_i32);
    CHECK(sys::result<std::vector<std::string_view>, std::initializer_list<std::string_view>>(2uz, "hallo"sv).expect() == std::vector<std::string_view> { "hallo"sv, "hallo"sv });
    CHECK(sys::result<i32, i32>(sys::error_tag, 123_i32).expect_err() == 123_i32);
    CHECK(sys::result<i32, std::string_view>("error_msg").expect_err() == "error_msg");
    CHECK(sys::result<i64, i32>(123_i32).expect_err() == 123_i32);
    CHECK(sys::result<const char*, std::string>(5uz, 'o').expect_err() == "ooooo");

    CHECK(sys::result<i32>(123_i32 /* NOLINT(readability-magic-numbers) */).expect() == 123_i32);
    CHECK(sys::result<std::vector<std::string_view>>(2uz, "hallo"sv).expect() == std::vector<std::string_view> { "hallo"sv, "hallo"sv });
    CHECK(sys::result<const char*>().expect() == nullptr);
    CHECK(sys::result<i64>(nullptr).operator!());

    CHECK(sys::result<void, const char*>());
    CHECK(sys::result<void, std::initializer_list<std::string_view>>());
    CHECK(sys::result<void, i32>(sys::error_tag, 123_i32).expect_err() == 123_i32);
    CHECK(sys::result<void, std::string_view>("error_msg").expect_err() == "error_msg");
    CHECK(sys::result<void, i32>(123_i32).expect_err() == 123_i32);
    CHECK(sys::result<void, std::string>(5uz, 'o').expect_err() == "ooooo");

    CHECK(sys::result<void>());
    CHECK(sys::result<void>(nullptr).operator!());
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::result(result other&&), other == true", "[sys][result]", (sys::result<i32, const char*>),
                                                                     (sys::result<i32>), (sys::result<void, const char*>), (sys::result<void>))
{
    TestType res = []() -> TestType
    {
        if constexpr (!std::same_as<typename TestType::value_type, void>)
            return 456_i32 /* NOLINT(readability-magic-numbers) */;
        else
            return {};
    }();

    if constexpr (!std::same_as<typename TestType::value_type, void>)
        CHECK(TestType(std::move(res) /* NOLINT(clang-analyzer-cplusplus.Move, bugprone-use-after-move) */).expect() == 456_i32);
    else
        CHECK(TestType(std::move(res)).operator bool()); // NOLINT(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
    CHECK_FALSE(TestType(std::move(res) /* NOLINT(clang-analyzer-cplusplus.Move, bugprone-use-after-move) */).operator bool());
    CHECK_FALSE(TestType(std::move(res) /* NOLINT(clang-analyzer-cplusplus.Move, bugprone-use-after-move) */).operator!());
}
TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::result(result other&&), other == false", "[sys][result]",
                                                                     (sys::result<std::string, std::string_view>), (sys::result<std::string>),
                                                                     (sys::result<void, std::string_view>), (sys::result<void>))
{
    TestType res = []() -> TestType
    {
        if constexpr (!std::same_as<typename TestType::err_type, void>)
            return "bad"sv;
        else
            return nullptr;
    }();

    if constexpr (!std::same_as<typename TestType::err_type, void>)
        CHECK(TestType(std::move(res) /* NOLINT(clang-analyzer-cplusplus.Move, bugprone-use-after-move) */).expect_err() == "bad"sv);
    else
        CHECK(TestType(std::move(res)).operator!()); // NOLINT(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
    CHECK_FALSE(TestType(std::move(res) /* NOLINT(clang-analyzer-cplusplus.Move, bugprone-use-after-move) */).operator bool());
    CHECK_FALSE(TestType(std::move(res) /* NOLINT(clang-analyzer-cplusplus.Move, bugprone-use-after-move) */).operator!());
}
TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::operator=(result&&), resFrom = std::move(resFrom)", "[sys][result]",
                                                                     (sys::result<i16, std::string>), (sys::result<i16>), (sys::result<void, std::string>), (sys::result<void>))
{
    TestType resFrom = []() -> TestType
    {
        if constexpr (!std::same_as<typename TestType::value_type, void>)
            return 4_i16;
        else
            return {};
    }();

    _nowarn_begin_one_gcc("-Wself-move");
    _nowarn_begin_one_clang(_clwarn_clang_self_move);
    CHECK((resFrom = std::move(resFrom)));
    _nowarn_end_gcc();
    _nowarn_end_clang();
}
TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::operator=(result&&), resFrom == true, res == false", "[sys][result]",
                                                                     (sys::result<i16, std::string>), (sys::result<i16>), (sys::result<void, std::string>), (sys::result<void>))
{
    TestType resFrom = []() -> TestType
    {
        if constexpr (!std::same_as<typename TestType::value_type, void>)
            return 5_i16 /* NOLINT(readability-magic-numbers) */;
        else
            return {};
    }();
    TestType res = []() -> TestType
    {
        if constexpr (!std::same_as<typename TestType::err_type, void>)
            return "never";
        else
            return nullptr;
    }();
    res = std::move(resFrom);

    CHECK(res);
    if constexpr (!std::same_as<typename TestType::value_type, void>)
        CHECK(res.expect() == 5_i16);
    else
        CHECK(res.operator bool());
}
TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::operator=(result&&), resFrom = false, res = true", "[sys][result]",
                                                                     (sys::result<i16, std::string>), (sys::result<i16>), (sys::result<void, std::string>), (sys::result<void>))
{
    TestType resFrom = []() -> TestType
    {
        if constexpr (!std::same_as<typename TestType::err_type, void>)
            return "always"sv;
        else
            return nullptr;
    }();
    TestType res = []() -> TestType
    {
        if constexpr (!std::same_as<typename TestType::value_type, void>)
            return 1_i16;
        else
            return {};
    }();
    res = std::move(resFrom);

    if constexpr (!std::same_as<typename TestType::err_type, void>)
        CHECK(res.expect_err() == "always");
    else
        CHECK(res.operator!());
}
TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::operator=(result&&), res(From).expect(_err)() => !res(From)", "[sys][result]",
                                                                     (sys::result<i16, std::string>), (sys::result<i16>), (sys::result<void, std::string>), (sys::result<void>))
{
    TestType resFrom = []() -> TestType
    {
        if constexpr (!std::same_as<typename TestType::err_type, void>)
            return "always"sv;
        else
            return nullptr;
    }();
    TestType res = []() -> TestType
    {
        if constexpr (!std::same_as<typename TestType::value_type, void>)
            return 1_i16;
        else
            return {};
    }();
    (void)resFrom.expect_err();
    (void)res.expect();

    CHECK_FALSE(resFrom.operator bool());
    CHECK_FALSE(resFrom.operator!());
    CHECK_FALSE(res.operator bool());
    CHECK_FALSE(res.operator!());
    res = std::move(resFrom);

    CHECK_FALSE(resFrom /* NOLINT(bugprone-use-after-move) */.operator bool());
    CHECK_FALSE(resFrom.operator!());
    CHECK_FALSE(res.operator bool());
    CHECK_FALSE(res.operator!());
}

TEST_CASE("result<T, U>::operator result<T, void>()", "[sys][result]")
{
    {
        sys::result<std::string, u32> res = "hallo again!";

        sys::result<std::string> got = _as(std::move(res), sys::result<std::string>);
        CHECK(got.expect() == "hallo again!");
    }

    {
        sys::result<std::string, std::string_view> res = "nooooooooo"sv;

        const sys::result<std::string> got = _as(std::move(res), sys::result<std::string>);
        CHECK(got.operator!());
    }
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::move(), result<T, U>::move_or(...), result<T, U>::expect()", "[sys][result]",
                                                                     (sys::result<i32, i32>), (sys::result<i32>))
{
    {
        TestType res = []() -> TestType
        {
            if constexpr (!std::same_as<typename TestType::value_type, void>)
                return 42_i32 /* NOLINT(readability-magic-numbers) */;
            else
                return {};
        }();

        CHECK(res.operator bool());
        CHECK(res.move() == 42_i32);
    }

    CHECK(TestType(42_i32).move_or(0_i32) == 42_i32);
    CHECK(TestType(42_i32).expect() == 42_i32);
}
TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::err(), result<T, U>::expect_err()", "[sys][result]", (sys::result<i64, i32>),
                                                                     (sys::result<void, i32>))
{
    {
        TestType res = []() -> TestType
        {
            if constexpr (!std::same_as<typename TestType::err_type, void>)
                return 42_i32 /* NOLINT(readability-magic-numbers) */;
            else
                return nullptr;
        }();

        CHECK(res.operator!());
        CHECK(res.err() == 42_i32);
    }

    CHECK(sys::result<i64, i32>(42_i32).move_or(0_i64) == 0_i64);
    CHECK(sys::result<i64, i32>(42_i32).expect_err() == 42_i32);
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::transform(...), res == true", "[sys][result]", (sys::result<std::string_view, i32>),
                                                                     (sys::result<std::string_view>), (sys::result<void, i32>), (sys::result<void>))
{
    if constexpr (!std::same_as<typename TestType::value_type, void>)
        CHECK(TestType("yippeeee").transform([](TestType&& res) -> std::string { return res ? std::string(std::move(res).move()) : "oops"; }) == "yippeeee");
    else
        CHECK(TestType().transform([](TestType&& res) -> std::string { return std::move(res) ? "yippeeee" : "oops"; }) == "yippeeee");
}
TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("result<T, U>::transform(...), res == false", "[sys][result]", (sys::result<std::string_view, i32>),
                                                                     (sys::result<std::string_view>), (sys::result<void, i32>), (sys::result<void>))
{

    if constexpr (!std::same_as<typename TestType::err_type, void>)
        CHECK(TestType(-1_i32).transform([](TestType&& res) -> std::string { return std::move(res) ? std::string(12, 'a') : "oops"; }) == "oops");
    else
        CHECK(TestType(nullptr).transform([](TestType&& res) -> std::string { return std::move(res) ? std::string(12, 'a') : "oops"; }) == "oops");
}

TEST_CASE("swap(result<T, U>&, result<T, U>&)", "[sys][result]")
{
    using std::swap;

    {
        sys::result<i8, std::string> res1 = 1_i8;
        sys::result<i8, std::string> res2 = "err";
        swap(res1, res2);

        const i8 gotVal = res2.expect();
        CHECK(gotVal == 1_i8);

        swap(res1, res2);

        const std::string gotErr = res2.expect_err();
        CHECK(gotErr == "err");
    }

    {
        sys::result<std::vector<i16>, i16> res1 = std::initializer_list { 1_i16 };
        sys::result<std::vector<i16>, i16> res2 = -3_i16;

        swap(res1, res2);

        const std::vector<i16> gotVal = res2.expect();
        CHECK(gotVal == std::vector<i16> { 1_i16 });

        swap(res1, res2);

        const i16 gotErr = res2.expect_err();
        CHECK(gotErr == -3_i16);
    }

    {
        sys::result<i8, std::string> res1 = 1_i8;
        sys::result<i8, std::string> res2 = "err";
        swap(res1, res2);
        swap(res1, res2);

        const i8 gotVal = res1.expect();
        CHECK(gotVal == 1_i8);
        const std::string gotErr = res2.expect_err();
        CHECK(gotErr == "err");
    }

    {
        sys::result<std::vector<i16>, i16> res1 = std::initializer_list { 1_i16 };
        sys::result<std::vector<i16>, i16> res2 = -3_i16;
        swap(res1, res2);
        swap(res1, res2);

        const std::vector<i16> gotVal = res1.expect();
        CHECK(gotVal == std::vector<i16> { 1_i16 });
        const i16 gotErr = res2.expect_err();
        CHECK(gotErr == -3_i16);
    }
}

TEST_CASE("result<void, void>", "[sys][result]")
{
    CHECK(sys::result<void>());
    CHECK(sys::result<void, void>(nullptr).operator!());

    {
        sys::result<void> res1;
        sys::result<void> res2 = nullptr;
        sys::result<void> res3 = std::move(res1);
        res1 = std::move(res2);
        res2 = std::move(res3);

        CHECK(res1.operator!());
        CHECK(res2);
        res1.expect_err();
        res2.expect();
    }

    CHECK(sys::result<void>(nullptr).transform([](sys::result<void>&& res /* NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved) */) -> i32
    { return res ? 0_i32 : -1_i32; }) == -1_i32);

    {
        sys::result<void> res1;
        sys::result<void> res2 = nullptr;
        swap(res1, res1);
        swap(res1, res2);

        CHECK(res1.operator!());
        CHECK(res2);
    }
}

TEST_CASE("result<T*, void>", "[sys][result]")
{
    using std::swap;
    int i = 0;

    {
        sys::result<int*> res1 = &i;
        sys::result<int*> res2 = nullptr;
        sys::result<int*> res3 = std::move(res1);
        res1 = std::move(res2);
        res2 = std::move(res3);

        CHECK_FALSE(res1);
        CHECK(res2);
    }

    {
        sys::result<int*> res = &i;
        CHECK(res);
        CHECK(res.move() == &i);
    }

    {
        int j = 0;
        sys::result<int*> res = &i;

        CHECK(res);
        CHECK(res.move_or(&j) == &i);
    }

    {
        int j = 0;
        sys::result<int*> res = nullptr;

        CHECK_FALSE(res);
        CHECK(res.move_or(&j) == &j);
    }

    CHECK(sys::result<int*>(&i).expect() == &i);
    sys::result<int*>(nullptr).expect_err();
    CHECK(sys::result<int*>(nullptr).transform([](sys::result<int*>&& res) -> i32 { return res ? i32(*std::move(res).move()) : -1_i32; }) == -1);

    {
        sys::result<int*> res1 = &i;
        sys::result<int*> res2 = nullptr;
        swap(res1, res1);
        swap(res1, res2);

        CHECK_FALSE(res1);
        CHECK(res2.expect() == &i);
    }
}

_nowarn_end_clang();

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
