#include <vector>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>
#include <module/sys.Threading>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Condition variable wait / signal.", "[sys.Threading][cond_var]")
{
    sys::cond_var cv;
    sys::mutex mtx;
    bool ready = false;

    sys::managed_thread t = sys::managed_thread::ctor([&]
    {
        const auto gRes = mtx.lock();
        REQUIRE(gRes);

        while (!ready)
            REQUIRE(cv.wait(mtx));
    }).move();

    {
        const auto gRes = mtx.lock();
        REQUIRE(gRes);
        ready = true;
    }
    REQUIRE(cv.notify_one());

    REQUIRE(t.join());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Condition variable broadcast wakes everyone.", "[sys.Threading][cond_var]")
{
    constexpr i32 numWaiters = 5;

    sys::cond_var cv;
    sys::mutex mtx;
    i32 count = 0;

    std::vector<sys::managed_thread> waiters;
    for (i32 i = 0; i < numWaiters; ++i)
    {
        waiters.emplace_back(sys::managed_thread::ctor([&]
        {
            const auto gRes = mtx.lock();
            REQUIRE(gRes);

            ++count;
            REQUIRE(cv.wait(mtx));
        }).move());
    }

    while (true)
    {
        const auto gRes = mtx.lock();
        REQUIRE(gRes);

        if (count == numWaiters)
            break;

        sys::thread_yield();
    }

    REQUIRE(cv.notify_all());
    waiters.clear();
}

TEST_CASE("Condition variable lazy initialization internally under contention.", "[sys.Threading][cond_var]")
{
    sys::cond_var cv;
    constexpr i32 numThreads = 20;
    std::vector<sys::managed_thread> threads;

    for (i32 i = 0; i < numThreads; ++i)
        threads.emplace_back(sys::managed_thread::ctor([&] { REQUIRE(cv.notify_one()); }).move());

    threads.clear();
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
