#include <vector>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Threading>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Condition variable wait / signal.", "[sys.Threading][cond_var]")
{
    sys::cond_var cv;
    sys::mutex mut;
    bool ready = false;

    sys::managed_thread t = sys::managed_thread::ctor([&]() -> void
    {
        const sys::mutex::guard g = mut.lock().expect();
        while (!ready)
            cv.wait(mut).expect();
    }).expect();

    {
        const sys::mutex::guard g = mut.lock().expect();
        ready = true;
    }
    cv.notify_one().expect();

    (void)t.join().expect();
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Condition variable broadcast wakes everyone.", "[sys.Threading][cond_var]")
{
    constexpr i32 numWaiters = 5;

    sys::cond_var cv;
    sys::mutex mut;
    i32 count = 0;

    std::vector<sys::managed_thread> waiters;
    for (i32 i = 0; i < numWaiters; ++i)
    {
        waiters.emplace_back(sys::managed_thread::ctor([&]() -> void
        {
            const sys::mutex::guard g = mut.lock().expect();
            ++count;
            cv.wait(mut).expect();
        }).expect());
    }

    while (true)
    {
        const sys::mutex::guard g = mut.lock().expect();
        if (count == numWaiters)
            break;
        sys::thread_yield();
    }

    cv.notify_all().expect();
    waiters.clear();
}

TEST_CASE("Condition variable lazy initialization under contention.", "[sys.Threading][cond_var]")
{
    sys::cond_var cv;
    constexpr i32 numThreads = 20;
    std::vector<sys::managed_thread> threads;

    for (i32 i = 0; i < numThreads; ++i)
        threads.emplace_back(sys::managed_thread::ctor([&]() -> void { cv.notify_one().expect(); }).expect());

    threads.clear();
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
