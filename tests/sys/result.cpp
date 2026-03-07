#include <memory>
#include <string>
#include <string_view>

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>

using namespace std::string_view_literals;

TEST_CASE("Constructing with a value works.", "[sys][result]")
{
    sys::result<i32, const char*> res = 123_i32; // NOLINT(readability-magic-numbers)
    CHECK(res);
    CHECK(res.move() == 123_i32);
}
TEST_CASE("Constructing with an error works.", "[sys][result]")
{
    sys::result<i32, std::string_view> res("error_msg");
    CHECK(!res);
    CHECK(res.err() == "error_msg");
}
TEST_CASE("Move semantics work correctly.", "[sys][result]")
{
    sys::result<i32, const char*> res1 = 456_i32; // NOLINT(readability-magic-numbers)
    sys::result<i32, const char*> res2 = std::move(res1);

    CHECK(res2);
    CHECK(res2.move() == 456_i32);
}
TEST_CASE("Swap works correctly.", "[sys][result]")
{
    sys::result<i32, std::string_view> res1 = 1_i32;
    sys::result<i32, std::string_view> res2 = "err"sv;

    using std::swap;
    swap(res1, res2);

    CHECK(!res1);
    CHECK(res2);
    CHECK(res2.move() == 1_i32);
    CHECK(res1.err() == "err");
}

TEST_CASE("Constructing any unit-valued-result works.", "[sys][result][unit]")
{
    sys::result<i32, void> res1 = 789_i32; // NOLINT(readability-magic-numbers)
    CHECK(res1);
    CHECK(res1.move() == 789_i32);

    CHECK(!sys::result<i32, void>(nullptr));

    CHECK(sys::result<void, i32>());

    sys::result<void, i32> res2 = 789_i32; // NOLINT(readability-magic-numbers)
    CHECK(!res2);
    CHECK(res2.err() == 789_i32);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Move semantics are sensible.")
{
    sys::result<i16> res1 = 1_i16;
    sys::result<i16> res2 = std::move(res1);

    CHECK(!_as(bool, res1)); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
    CHECK(!!res1);
    CHECK(res2);
    CHECK(!!res2);
    CHECK(res2.move() == 1_i16);

    i16 val = 2_i16;
    sys::result<i16&, i16> res3 = val;
    sys::result<i16&, i16> res4 = std::move(res3);
    CHECK(!_as(bool, res3)); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
    CHECK(!!res3);
    CHECK(res4);
    CHECK(!!res4);
    CHECK(std::addressof(res4.move()) == std::addressof(val));

    sys::result<void, i16> res5 = 3_i16;
    sys::result<void, i16> res6 = std::move(res5);
    CHECK(!_as(bool, res5)); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
    CHECK(!!res5);
    CHECK(!_as(bool, res6));
    CHECK(!res6);
    CHECK(res6.err() == 3_i16);

    sys::result<i16, std::string> res7 = 4_i16;
    sys::result<i16, std::string> res8 = std::move(res7);
    CHECK(!_as(bool, res7)); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
    CHECK(!!res7);
    CHECK(res8);
    CHECK(!!res8);
    CHECK(res8.move() == 4_i16);
}

TEST_CASE("Macros work as expected.", "[sys][result][macros]")
{
    auto testFunc = [](bool success) -> sys::result<i16, i32>
    {
        sys::result<i16, i32> res = success ? sys::result<i16, i32>(10_i16) : sys::result<i16, i32>(404_i32); // NOLINT(readability-magic-numbers)
        _res_movret(const i16 val, std::move(res));
        return { val + 1_i16 };
    };

    CHECK(testFunc(true).move() == 11_i16);
    CHECK(testFunc(false).err() == 404_i32);
}

TEST_CASE("Macros work with unit-result in non-template context.", "[sys][result][macros]")
{
    auto testUnitFunc = [](bool success) -> sys::result<i16>
    {
        sys::result<i16> res = success ? sys::result<i16>(10_i16) : sys::result<i16>(nullptr); // NOLINT(readability-magic-numbers)
        _res_movret(const i16 val, std::move(res));
        return { val + 1_i16 };
    };

    CHECK(testUnitFunc(true).move() == 11_i16);
    CHECK(!testUnitFunc(false));
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Value/error type semantics are correct.", "[sys][result]")
{
    CHECK([]() -> sys::result<void> { return {}; }());
    CHECK(![]() -> sys::result<void> { return nullptr; }());
    CHECK([]() -> sys::result<i32, i32> { return 4_i32; }().move() == 4_i32);
    CHECK([]() -> sys::result<i32, i32> { return { sys::error_tag(), 4_i32 }; }().err() == 4_i32);
    CHECK([]() -> sys::result<i32> { return 4_i32; }().move() == 4_i32);
    CHECK([]() -> sys::result<i32> { return nullptr; }().move_or(-1_i32) == -1_i32);
    CHECK([]() -> sys::result<void, i32> { return {}; }());
    CHECK(![]() -> sys::result<void, i32> { return 4_i32; }());

    CHECK([]() -> sys::result<std::string, const char*> { return "hallo"; }().move() == "hallo");
    CHECK(std::string_view([]() -> sys::result<std::string, const char*> { return _as(const char*, "hallo"); }().err()) == "hallo");
    CHECK([]() -> sys::result<std::string, const char*> { return { 4uz, 'a' }; }().move() == "aaaa");
    CHECK(std::string_view([]() -> sys::result<std::string, const char*> { return { sys::error_tag(), "oops" }; }().err()) == "oops");

    CHECK([]() -> sys::result<ullong, long> { return 10293948287ull; }().move() == 10293948287ull);
    CHECK([]() -> sys::result<ullong, long> { return 10293948287; }().move() == 10293948287ull);
    CHECK([]() -> sys::result<ullong, long> { return 12l; }().err() == 12);
    CHECK([]() -> sys::result<ullong, long> { return { sys::error_tag(), 12 }; }().err() == 12);
}

TEST_CASE("Value type can be reference.", "[sys][result]")
{
    const std::string str = "interesting";

    const std::string& val = sys::result<const std::string&>(str).move();
    CHECK(std::addressof(val) == std::addressof(str));
    CHECK(str == val);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Can transform result to any value.", "[sys][result][transform]")
{
    CHECK(sys::result<i64>(nullptr).transform([](sys::result<i64>&& res) { return res ? std::move(res).move() : 24_i64; }) == 24_i64);
    CHECK(sys::result<i64, i32>(23_i32).transform([](sys::result<i64, i32>&& res) { return res ? std::move(res).move() : 24_i64; }) == 24_i64);
    CHECK(sys::result<i64, i32>(23_i64).transform([](sys::result<i64, i32>&& res) { return res ? std::move(res).move() : 24_i64; }) == 23_i64);
}

// NOLINTEND(misc-include-cleaner)
