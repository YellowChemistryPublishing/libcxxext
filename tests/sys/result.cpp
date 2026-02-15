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
    const sys::result<i32, std::string_view> res("error_msg");
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

TEST_CASE("Constructing unit-result with a value works.", "[sys][result][unit]")
{
    sys::result<i32, void> res = 789_i32; // NOLINT(readability-magic-numbers)
    CHECK(res);
    CHECK(res.move() == 789_i32);
}
TEST_CASE("Constructing with `nullptr` (error) works.", "[sys][result][unit]")
{
    const sys::result<i32, void> res = nullptr;
    CHECK(!res);
}

TEST_CASE("Macros work as expected.", "[sys][result][macros]")
{
    auto testFunc = [](bool success) -> sys::result<i16, i32>
    {
        sys::result<i16, i32> res = success ? sys::result<i16, i32>(10_i16) : sys::result<i16, i32>(404_i32); // NOLINT(readability-magic-numbers)
        i16 val = 0_i16;
        _res_movret(val, res);
        return { val + 1_i16 };
    };

    CHECK(testFunc(true).move() == 11_i16);
    CHECK(testFunc(false).err() == 404_i32);
}

// NOLINTEND(misc-include-cleaner)
