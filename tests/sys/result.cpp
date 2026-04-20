#include <memory>
#include <string>
#include <string_view>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>

using namespace std::string_view_literals;

TEST_CASE("Constructing with a value works.", "[sys][result]")
{
    CHECK(sys::result<i32, const char*>(123_i32).expect() == 123_i32); // NOLINT(readability-magic-numbers)
}
TEST_CASE("Constructing with an error works.", "[sys][result]") { CHECK(sys::result<i32, std::string_view>("error_msg").expect_err() == "error_msg"); }
TEST_CASE("Move semantics work correctly.", "[sys][result]")
{
    sys::result<i32, const char*> res1 = 456_i32; // NOLINT(readability-magic-numbers)
    sys::result<i32, const char*> res2 = std::move(res1);

    const i32 val = res2.expect();
    CHECK(val == 456_i32);
}
TEST_CASE("Swap works correctly.", "[sys][result]")
{
    sys::result<i32, std::string_view> res1 = 1_i32;
    sys::result<i32, std::string_view> res2 = "err"sv;

    using std::swap;
    swap(res1, res2);

    const i32 val = res2.expect();
    CHECK(val == 1_i32);
    const std::string_view err = res1.expect_err();
    CHECK(err == "err");
}

TEST_CASE("Constructing any unit-valued-result works.", "[sys][result][unit]")
{
    sys::result<i32, void> res1 = 789_i32; // NOLINT(readability-magic-numbers)
    i32 val = res1.expect();
    CHECK(val == 789_i32);

    sys::result<i32, void>(nullptr).expect_err();
    sys::result<void, i32>().expect();

    sys::result<void, i32> res2 = 789_i32; // NOLINT(readability-magic-numbers)
    i32 err = res2.expect_err();
    CHECK(err == 789_i32);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Move semantics are sensible.")
{
    sys::result<i16> res1 = 1_i16;
    sys::result<i16> res2 = std::move(res1);

    CHECK_FALSE(res1.operator bool()); // NOLINT(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
    CHECK(!!res1);                     // NOLINT(clang-analyzer-cplusplus.Move)

    bool valid = res2.operator bool();
    CHECK(valid);

    CHECK(!!res2);

    i16 val = res2.expect();
    CHECK(val == 1_i16);

    val = 2_i16;
    sys::result<i16&, i16> res3 = val;
    sys::result<i16&, i16> res4 = std::move(res3);
    CHECK_FALSE(res3.operator bool()); // NOLINT(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
    CHECK(!!res3);                     // NOLINT(clang-analyzer-cplusplus.Move)

    valid = res4.operator bool();
    CHECK(valid);

    CHECK(!!res4);
    valid = std::addressof(res4.expect()) == std::addressof(val);
    CHECK(valid);

    sys::result<void, i16> res5 = 3_i16;
    sys::result<void, i16> res6 = std::move(res5);
    CHECK_FALSE(res5.operator bool()); // NOLINT(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
    CHECK(!!res5);                     // NOLINT(clang-analyzer-cplusplus.Move)
    CHECK_FALSE(res6.operator bool());

    const i16 err = res6.expect_err();
    CHECK(err == 3_i16);

    sys::result<i16, std::string> res7 = 4_i16;
    sys::result<i16, std::string> res8 = std::move(res7);
    CHECK_FALSE(res7.operator bool()); // NOLINT(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
    CHECK(!!res7);                     // NOLINT(clang-analyzer-cplusplus.Move)
    CHECK(!!res8);

    valid = res8.operator bool();
    CHECK(valid);

    val = res8.expect();
    CHECK(val == 4_i16);
}

TEST_CASE("Macros work as expected.", "[sys][result][macros]")
{
    auto testFunc = [](bool success) -> sys::result<i16, i32>
    {
        sys::result<i16, i32> res = success ? sys::result<i16, i32>(10_i16) : sys::result<i16, i32>(404_i32); // NOLINT(readability-magic-numbers)
        _res_movret(const i16 val, std::move(res));
        return { val + 1_i16 };
    };

    CHECK(testFunc(true).expect() == 11_i16);
    CHECK(testFunc(false).expect_err() == 404_i32);
}

TEST_CASE("Macros work with unit-result in non-template context.", "[sys][result][macros]")
{
    auto testUnitFunc = [](bool success) -> sys::result<i16>
    {
        sys::result<i16> res = success ? sys::result<i16>(10_i16) : sys::result<i16>(nullptr); // NOLINT(readability-magic-numbers)
        _res_movret(const i16 val, std::move(res));
        return { val + 1_i16 };
    };

    CHECK(testUnitFunc(true).expect() == 11_i16);
    testUnitFunc(false).expect_err();
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Value/error type semantics are correct.", "[sys][result]")
{
    []() -> sys::result<void> { return {}; }().expect();
    CHECK_FALSE([]() -> sys::result<void> { return nullptr; }());
    CHECK([]() -> sys::result<i32, i32> { return 4_i32; }().expect() == 4_i32);
    CHECK([]() -> sys::result<i32, i32> { return { sys::error_tag, 4_i32 }; }().expect_err() == 4_i32);
    CHECK([]() -> sys::result<i32> { return 4_i32; }().expect() == 4_i32);
    CHECK([]() -> sys::result<i32> { return nullptr; }().move_or(-1_i32) == -1_i32);
    []() -> sys::result<void, i32> { return {}; }().expect();
    CHECK_FALSE([]() -> sys::result<void, i32> { return 4_i32; }().operator bool());

    CHECK([]() -> sys::result<std::string, const char*> { return "hallo"; }().expect() == "hallo");
    CHECK(std::string_view([]() -> sys::result<std::string, const char*> { return _as(const char*, "hallo"); }().expect_err()) == "hallo");
    CHECK([]() -> sys::result<std::string, const char*> { return { 4uz, 'a' }; }().expect() == "aaaa");
    CHECK(std::string_view([]() -> sys::result<std::string, const char*> { return { sys::error_tag, "oops" }; }().expect_err()) == "oops");

    CHECK([]() -> sys::result<u64, i16> { return 10293948287_u64; }().expect() == 10293948287_u64);
    CHECK([]() -> sys::result<u64, i16> { return 10293948287_i64; }().expect() == 10293948287_u64);
    CHECK([]() -> sys::result<u64, i16> { return 12_i16; }().expect_err() == 12_i16);
    CHECK([]() -> sys::result<u64, i16> { return { sys::error_tag, 12_u64 }; }().expect_err() == 12_i16);
}

TEST_CASE("Value type can be reference.", "[sys][result]")
{
    const std::string str = "interesting";

    _nowarn_begin_one_gcc("-Wdangling-reference");
    const std::string& val = sys::result<const std::string&>(str).expect();
    _nowarn_end_gcc();

    CHECK(std::addressof(val) == std::addressof(str));
    CHECK(str == val);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Can transform result to any value.", "[sys][result][transform]")
{
    i64 val = sys::result<i64>(nullptr).transform([](sys::result<i64>&& res) -> i64 { return std::move(res).move_or(24_i64 /* NOLINT(readability-magic-numbers) */); });
    CHECK(val == 24_i64);
    val = sys::result<i64, i32>(23_i32).transform([](sys::result<i64, i32>&& res) -> i64 { return std::move(res).move_or(24_i64 /* NOLINT(readability-magic-numbers) */); });
    CHECK(val == 24_i64);
    val = sys::result<i64, i32>(23_i64).transform([](sys::result<i64, i32>&& res) -> i64 { return std::move(res).move_or(24_i64 /* NOLINT(readability-magic-numbers) */); });
    CHECK(val == 23_i64);
}

TEST_CASE("Can convert result to unit-result.", "[sys][result]")
{
    CHECK(_as(sys::result<void>, sys::result<void>()));
    CHECK_FALSE(_as(sys::result<void>, sys::result<void, i32>(4_i32)));
    _as(sys::result<i64>, sys::result<i64>(nullptr)).expect_err();
    CHECK_FALSE(_as(bool, _as(sys::result<i64>, sys::result<i64, i32>(23_i32))));
    CHECK(_as(sys::result<i64>, sys::result<i64, i32>(23_i64)).expect() == 23_i64);
}

TEST_CASE("Pointer result (analogous to any other nullable-value result) works correctly.", "[sys][result]")
{
    sys::result<int*> res = nullptr;
    CHECK_FALSE(res);
    CHECK(!res);
    CHECK(res.move_or(nullptr) == nullptr);

    int i = 0;
    res = sys::result<int*>(&i);
    CHECK(res);
    CHECK(res.move() == &i);

    res = sys::result<int*>(&i);
    CHECK(std::move(res) /* NOLINT(bugprone-use-after-move) */.transform([](sys::result<int*>&& p) -> int { return *std::move(p).move(); }) == 0);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
