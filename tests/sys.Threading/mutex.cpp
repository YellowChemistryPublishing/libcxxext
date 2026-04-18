#include <mutex>
#include <vector>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Threading>

TEST_CASE("Mutex guard acquisition.", "[sys.Threading][mutex]")
{
    sys::mutex mut;
    (void)mut.try_lock().expect();
    const sys::mutex::guard g2 = mut.lock().expect();
    CHECK(mut.try_lock().expect_err() == sys::threading_error::busy);
}

TEST_CASE("Mutex direct acquire/release.", "[sys.Threading][mutex]")
{
    sys::mutex mut;
    mut.try_acquire(unsafe).expect();
    mut.release(unsafe).expect();
    mut.acquire(unsafe).expect();
    CHECK(mut.try_acquire(unsafe).expect_err() == sys::threading_error::busy);
}

TEST_CASE("Mutex guard is movable.", "[sys.Threading][mutex]")
{
    sys::mutex mut;
    sys::mutex::guard g1 = mut.lock().expect();
    const sys::mutex::guard g2 = std::move(g1);
}

TEST_CASE("Mutex actually guards under contention.", "[sys.Threading][mutex]")
{
    constexpr i32 numThreads = 20;
    constexpr i32 incrementsPerThread = 10000;

    sys::mutex mut;
    std::mutex referenceMtx;
    std::vector<sys::managed_thread> threads;

    for (i32 i = 0; i < numThreads; i++)
    {
        threads.emplace_back(sys::managed_thread::ctor([&]() -> void
        {
            for (i32 j = 0; j < incrementsPerThread; j++)
            {
                const sys::mutex::guard g = mut.lock().expect();
                CHECK(referenceMtx.try_lock());
                referenceMtx.unlock();
            }
        }).expect());
    }
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
