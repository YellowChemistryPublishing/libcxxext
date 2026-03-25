#include <mutex>
#include <vector>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys>
#include <module/sys.Threading>

TEST_CASE("Mutex guard acquisition.", "[sys.Threading][mutex]")
{
    sys::mutex mtx;
    CHECK(mtx.try_lock());

    auto gRes = mtx.lock();
    REQUIRE(gRes);

    const sys::mutex::guard g = gRes.move();
    CHECK(mtx.try_lock().err() == sys::threading_error::operation_failed);
}

TEST_CASE("Mutex guard is movable.", "[sys.Threading][mutex]")
{
    sys::mutex mtx;
    auto gRes = mtx.lock();
    REQUIRE(gRes);

    sys::mutex::guard g1 = gRes.move();
    const sys::mutex::guard g2 = std::move(g1);
}

TEST_CASE("Mutex actually guards under contention.", "[sys.Threading][mutex]")
{
    constexpr i32 numThreads = 20;
    constexpr i32 incrementsPerThread = 10000;

    sys::mutex mtx;
    std::mutex referenceMtx;
    std::vector<sys::thread_handle> threads;

    for (i32 i = 0; i < numThreads; i++)
    {
        threads.emplace_back(sys::thread_handle::ctor([&]
        {
            for (i32 j = 0; j < incrementsPerThread; j++)
            {
                auto gRes = mtx.lock();
                REQUIRE(gRes);

                const sys::mutex::guard g = gRes.move();
                CHECK(referenceMtx.try_lock());
                referenceMtx.unlock();
            }
        }).move());
    }
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
