#include <string_view>

// NOLINTBEGIN(misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>

using namespace std::string_view_literals;

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Basic result with error type works as expected.", "[sys][result]")
{
    SECTION("Constructing with a value works.")
    {
        sys::result<i32, const char*> res = 123_i32; // NOLINT(readability-magic-numbers)
        CHECK(res);
        CHECK(res.move() == 123_i32);
    }

    SECTION("Constructing with an error works.")
    {
        const sys::result<i32, std::string_view> res("error_msg");
        CHECK(!res);
        CHECK(res.err() == "error_msg");
    }

    SECTION("Move semantics work correctly.")
    {
        sys::result<i32, const char*> res1 = 456_i32; // NOLINT(readability-magic-numbers)
        sys::result<i32, const char*> res2 = std::move(res1);

        CHECK(res2);
        CHECK(res2.move() == 456_i32);
    }

    SECTION("Swap works correctly.")
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
}

TEST_CASE("Result with void error type works as expected.", "[sys][result][unit]")
{
    SECTION("Constructing with a value works.")
    {
        sys::result<i32, void> res = 789_i32; // NOLINT(readability-magic-numbers)
        CHECK(res);
        CHECK(res.move() == 789_i32);
    }

    SECTION("Constructing with nullptr (error) works.")
    {
        const sys::result<i32, void> res = nullptr;
        CHECK(!res);
    }
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
