#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Threading>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Once executes exactly once sequentially.", "[sys.Threading][once]")
{
    sys::once o;
    i32 counter = 0_i32;

    CHECK_FALSE(o.is_completed());
    CHECK_NOTHROW(o.call_once([&]() -> void { ++counter; }));
    CHECK(o.is_completed());
    CHECK(counter == 1_i32);

    CHECK_NOTHROW(o.call_once([&]() -> void { ++counter; }));
    CHECK(counter == 1_i32);
    CHECK(o.is_completed());
}

TEST_CASE("Once correctly forwards arguments.", "[sys.Threading][once]")
{
    sys::once o;
    i32 res = 0_i32;

    o.call_once([&](const i32 a, const i32 b) -> void
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
        res = a + b;
    }, 10_i32 /* NOLINT(readability-magic-numbers) */, 20_i32 /* NOLINT(readability-magic-numbers) */);
    CHECK(res == 30_i32);
    CHECK(o.is_completed());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Once handles exceptions and allows retry.", "[sys.Threading][once]")
{
    sys::once o;
    i32 count = 0_i32;

    auto func = [&]() -> i32
    {
        if (++count == 1_i32)
            throw std::runtime_error("uh oh");
        return 42_i32 /* NOLINT(readability-magic-numbers) */;
    };

    CHECK_THROWS_AS(o.call_once(func), std::runtime_error);
    CHECK_FALSE(o.is_completed());
    CHECK(count == 1_i32);

    CHECK_NOTHROW(o.call_once(func));
    CHECK(o.is_completed());
    CHECK(count == 2_i32);

    o.call_once(func);
    CHECK(count == 2_i32);
}

TEST_CASE("Once actually guards under contention.", "[sys.Threading][once]")
{
    constexpr int numThreads = 160;

    sys::once o;
    std::atomic<int> runCount = 0;
    std::atomic<int> readyCount = 0;
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

    std::vector<sys::managed_thread> threads;
    for (sz i = 0_uz; i < numThreads; i++)
    {
        threads.emplace_back(sys::managed_thread::ctor([&]() -> void
        {
            ++readyCount;
            while (!flag.test(std::memory_order_acquire))
                sys::thread_yield();

            o.call_once([&]() -> void
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
                ++runCount;
            });
            CHECK(o.is_completed());
        }).expect());
    }

    while (readyCount.load(std::memory_order_acquire) != numThreads)
        sys::thread_yield();
    flag.test_and_set(std::memory_order_release);

    threads.clear();
    CHECK(runCount.load() == 1);
    CHECK(o.is_completed());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Once wait successfully blocks.", "[sys.Threading][once]")
{
    sys::once o;
    std::atomic_flag started = ATOMIC_FLAG_INIT;
    std::atomic_flag done = ATOMIC_FLAG_INIT;

    const sys::managed_thread thread = sys::managed_thread::ctor([&]() -> void
    {
        o.call_once([&]() -> void
        {
            started.test_and_set(std::memory_order_release);
            while (!done.test(std::memory_order_acquire))
                sys::thread_yield();
        });
    }).expect();
    while (!started.test(std::memory_order_acquire))
        sys::thread_yield();

    CHECK_FALSE(o.is_completed());

    std::atomic_flag waitFinished = ATOMIC_FLAG_INIT;
    const sys::managed_thread waiterThread = sys::managed_thread::ctor([&]() -> void
    {
        o.wait();
        o.wait(); // Should return immediately.
        waitFinished.test_and_set(std::memory_order_release);
        CHECK(o.is_completed());
    }).expect();

    std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
    CHECK_FALSE(waitFinished.test(std::memory_order_acquire));

    done.test_and_set(std::memory_order_release);
    while (!waitFinished.test(std::memory_order_acquire))
        sys::thread_yield();

    CHECK(waitFinished.test(std::memory_order_acquire));
    CHECK(o.is_completed());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Once correctly handles `sys::result<...>` return types.", "[sys.Threading][once]")
{
    sys::once o;
    i32 count = 0_i32;

    const auto func = [&](const bool succeed) -> sys::result<i64, i32>
    {
        ++count;
        return succeed ? sys::result<i64, i32>(100_i64) : sys::result<i64, i32>(42_i32); // NOLINT(readability-magic-numbers)
    };

    auto res1 = o.call_once(func, false);
    i32 err = res1.expect_err();
    CHECK(err == 42_i32);
    CHECK_FALSE(o.is_completed());
    CHECK(count == 1_i32);

    auto res2 = o.call_once(func, false);
    err = res2.expect_err();
    CHECK(err == 42_i32);
    CHECK_FALSE(o.is_completed());
    CHECK(count == 2_i32);

    auto res3 = o.call_once(func, true);
    res3.expect();
    CHECK(o.is_completed());
    CHECK(count == 3_i32);

    auto res4 = o.call_once(func, false);
    res4.expect();
    CHECK(o.is_completed());
    CHECK(count == 3_i32);
}
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Once correctly handles void-as-error result types.", "[sys.Threading][once]")
{
    sys::once o;
    i32 count = 0_i32;

    const auto func = [&](const bool succeed) -> sys::result<void>
    {
        ++count;
        return succeed ? sys::result<void>() : sys::result<void>(nullptr);
    };

    auto res1 = o.call_once(func, false);
    CHECK_FALSE(res1);
    CHECK(!res1);
    CHECK_FALSE(o.is_completed());
    CHECK(count == 1_i32);

    auto res2 = o.call_once(func, false);
    CHECK_FALSE(res2);
    CHECK(!res2);
    CHECK_FALSE(o.is_completed());
    CHECK(count == 2_i32);

    auto res3 = o.call_once(func, true);
    CHECK(res3);
    CHECK(o.is_completed());
    CHECK(count == 3_i32);

    auto res4 = o.call_once(func, false);
    CHECK(res4);
    CHECK(o.is_completed());
    CHECK(count == 3_i32);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
