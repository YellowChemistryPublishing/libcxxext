#include <atomic>
#include <chrono>
#include <thread>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)

#include <catch2/catch_all.hpp>

#include <module/sys.Threading>

TEST_CASE("Threads run code.", "[sys.Threading][thread]")
{
    bool done = false;
    sys::thread thread = sys::thread::ctor([&] { done = true; }).move();

    REQUIRE(thread.join());
    CHECK_FALSE(thread.joinable());
    CHECK(done);
}

TEST_CASE("Thread joining returns value.", "[sys.Threading][thread]")
{
    sys::thread thread = sys::thread::ctor([]() -> int { return 42 /* NOLINT(readability-magic-numbers) */; }).move();
    CHECK(thread.join().move() == 42);
}

TEST_CASE("Thread handle is movable.", "[sys.Threading][thread]")
{
    sys::thread t1 = sys::thread::ctor([] { }).move();
    CHECK(t1.joinable());

    sys::thread t2 = std::move(t1);
    CHECK_FALSE(t1.joinable()); // NOLINT(bugprone-use-after-move)
    CHECK(t2.joinable());

    sys::thread t3;
    t3 = std::move(t2);
    CHECK_FALSE(t2.joinable()); // NOLINT(bugprone-use-after-move)
    CHECK(t3.joinable());
}

TEST_CASE("Thread handle joins on destruction.", "[sys.Threading][thread]")
{
    bool done = false;

    {
        const sys::thread thread = sys::thread::ctor([&]
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
        sys::thread thread = sys::thread::ctor([&]
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
    sys::thread thread = sys::thread::ctor(global_thread_func).move();
    CHECK(thread.join().move() == 123);
    CHECK(func_invoked);
}

TEST_CASE("Exception doesn't obliterate program.", "[sys.Threading][thread]")
{
    sys::thread thread = sys::thread::ctor([]
    {
        throw 42; // NOLINT(hicpp-exception-baseclass, readability-magic-numbers)
    }).move();
    CHECK(thread.join().move() == sys::bsentinel<int>());
}

TEST_CASE("Obtain current thread and id.", "[sys.Threading][thread]")
{
    const sys::thread cur = sys::thread_current();
    CHECK_FALSE(cur.joinable());

    const sys::thread_id id1 = cur.id();
    const sys::thread_id id2 = sys::thread_current().id();
    CHECK(id1 == id2);
    CHECK_FALSE(id1 == sys::thread_id(nullptr));

    sys::thread t = sys::thread::ctor([] { return 42 /* NOLINT(readability-magic-numbers) */; }).move();
    CHECK(t.joinable());
    CHECK_FALSE(t.id() == id1);

    REQUIRE(t.join());
}

TEST_CASE("Yield current thread.", "[sys.Threading][thread]")
{
    sys::thread_yield();

    std::atomic_flag done = ATOMIC_FLAG_INIT;
    sys::thread t = sys::thread::ctor([&]
    {
        while (!done.test())
            sys::thread_yield();
    }).move();

    std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
    done.test_and_set();
    REQUIRE(t.join());
}

TEST_CASE("Exit current thread.", "[sys.Threading][thread]")
{
    sys::thread t = sys::thread::ctor([]
    {
        sys::thread_exit(123 /* NOLINT(readability-magic-numbers) */);
        REQUIRE(false);
    }).move();

    CHECK(t.join().move() == 123 /* NOLINT(readability-magic-numbers) */);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
