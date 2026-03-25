#include <atomic>
#include <chrono>
#include <thread>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys.Threading>

TEST_CASE("Threads run code.", "[sys.Threading][thread_handle]")
{
    bool done = false;
    sys::thread_handle thread = sys::thread_handle::ctor([&] { done = true; }).move();

    REQUIRE(thread.join());
    CHECK_FALSE(thread.joinable());
    CHECK(done);
}

TEST_CASE("Thread joining returns value.", "[sys.Threading][thread_handle]")
{
    sys::thread_handle thread = sys::thread_handle::ctor([]() -> int { return 42 /* NOLINT(readability-magic-numbers) */; }).move();
    CHECK(thread.join().move() == 42);
}

TEST_CASE("Thread handle is movable.", "[sys.Threading][thread_handle]")
{
    sys::thread_handle t1 = sys::thread_handle::ctor([] { }).move();
    CHECK(t1.joinable());

    sys::thread_handle t2 = std::move(t1);
    CHECK_FALSE(t1.joinable()); // NOLINT(bugprone-use-after-move)
    CHECK(t2.joinable());

    sys::thread_handle t3;
    t3 = std::move(t2);
    CHECK_FALSE(t2.joinable()); // NOLINT(bugprone-use-after-move)
    CHECK(t3.joinable());
}

TEST_CASE("Thread handle joins on destruction.", "[sys.Threading][thread]")
{
    bool done = false;

    {
        const sys::thread_handle thread = sys::thread_handle::ctor([&]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
            done = true;
        }).move();
    } // Joinable `sys::thread_handle` joins on destruction.

    CHECK(done);
}

TEST_CASE("Thread handle can be detached.", "[sys.Threading][thread]")
{
    std::atomic_flag done = ATOMIC_FLAG_INIT;

    {
        sys::thread_handle thread = sys::thread_handle::ctor([&]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
            done.test_and_set();
        }).move();
        thread.detach();

        CHECK_FALSE(thread.joinable());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100 /* NOLINT(readability-magic-numbers) */));
    CHECK(done.test());
}

static bool func_invoked = false; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static int global_thread_func()
{
    func_invoked = true;
    return 123; // NOLINT(readability-magic-numbers)
}

TEST_CASE("Thread can take func ref/ptr.", "[sys.Threading][thread]")
{
    sys::thread_handle thread = sys::thread_handle::ctor(global_thread_func).move();
    CHECK(thread.join().move() == 123);
    CHECK(func_invoked);
}

TEST_CASE("Exception doesn't obliterate program.", "[sys.Threading][thread]")
{
    sys::thread_handle thread = sys::thread_handle::ctor([]
    {
        throw 42; // NOLINT(hicpp-exception-baseclass, readability-magic-numbers)
    }).move();
    CHECK(thread.join().move() == sys::bsentinel<int>());
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
