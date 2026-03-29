#include <atomic>
#include <chrono>
#include <thread>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys.Threading>

TEST_CASE("Threads run code.", "[sys.Threading][thread]")
{
    bool done = false;
    sys::managed_thread t = sys::managed_thread::ctor([&]() -> void { done = true; }).move();

    CHECK(t.joinable());
    REQUIRE(t.join());
    CHECK_FALSE(t.joinable());
    CHECK(done);
}

TEST_CASE("Thread joining returns value.", "[sys.Threading][thread]")
{
    sys::managed_thread t = sys::managed_thread::ctor([]() -> int { return 42 /* NOLINT(readability-magic-numbers) */; }).move();
    CHECK(t.join().move() == 42);
}

TEST_CASE("Thread handle is movable.", "[sys.Threading][thread]")
{
    sys::managed_thread t1 = sys::managed_thread::ctor([]() -> void { }).move();
    CHECK(t1.joinable());

    sys::managed_thread t2 = std::move(t1);
    CHECK_FALSE(t1.joinable()); // NOLINT(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
    CHECK(t2.joinable());

    sys::managed_thread t3 = nullptr;
    t3 = std::move(t2);
    CHECK_FALSE(t2.joinable()); // NOLINT(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
    CHECK(t3.joinable());
}

TEST_CASE("Thread handle joins on destruction.", "[sys.Threading][thread]")
{
    bool done = false;

    {
        const sys::managed_thread t = sys::managed_thread::ctor([&]() -> void
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
            done = true;
        }).move();
    } // Joinable `sys::thread` joins on destruction.

    CHECK(done);
}

TEST_CASE("Thread handle can be detached.", "[sys.Threading][thread]")
{
    std::atomic_flag done = ATOMIC_FLAG_INIT;

    {
        sys::managed_thread t = sys::managed_thread::ctor([&]() -> void
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
            done.test_and_set();
        }).move();
        t.detach();

        CHECK_FALSE(t.joinable());
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
    sys::managed_thread t = sys::managed_thread::ctor(global_thread_func).move();
    CHECK(t.join().move() == 123);
    CHECK(func_invoked);
}

TEST_CASE("Exception doesn't obliterate program.", "[sys.Threading][thread]")
{
    sys::managed_thread t = sys::managed_thread::ctor([]() -> void
    {
        throw 42; // NOLINT(hicpp-exception-baseclass, readability-magic-numbers)
    }).move();
    CHECK(t.join().move() == sys::bsentinel<int>());
}

TEST_CASE("Obtain current thread and id.", "[sys.Threading][thread]")
{
    const sys::thread cur = sys::thread_current();

    const sys::thread_id id1 = cur.id();
    const sys::thread_id id2 = sys::thread_current().id();
    CHECK(id1 == id2);
    CHECK_FALSE(id1 == sys::thread_id(nullptr));

    sys::managed_thread t = sys::managed_thread::ctor([]() -> int { return 42 /* NOLINT(readability-magic-numbers) */; }).move();
    CHECK(t.joinable());
    CHECK_FALSE(t.thread().id() == id1);

    REQUIRE(t.join());
}

TEST_CASE("Yield current thread.", "[sys.Threading][thread]")
{
    sys::thread_yield();

    std::atomic_flag done = ATOMIC_FLAG_INIT;
    sys::managed_thread t = sys::managed_thread::ctor([&]() -> void
    {
        while (!done.test())
            sys::thread_yield();
    }).move();

    std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
    done.test_and_set();
    REQUIRE(t.join());
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
