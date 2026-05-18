#include <chrono>
#include <thread>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys.Threading>

TEST_CASE("managed_thread", "[sys.Threading][managed_thread]")
{
    bool gotComplete = false;
    sys::managed_thread t1 = sys::managed_thread::ctor([&]() -> void
    {
        gotComplete = true;
        throw std::runtime_error("oops");
    }).expect();

    CHECK(t1);
    CHECK(t1.joinable());
    CHECK(t1.join().expect() == sys::bsentinel<int>());
    CHECK_FALSE(t1.joinable());
    CHECK(t1.join().expect_err() == sys::threading_error::invalid_operation);
    t1.detach(); // Must still work!
    CHECK(gotComplete);

    sys::managed_thread t2 = std::move(t1);

    CHECK_FALSE(t2);
    CHECK_FALSE(t2.joinable());
    CHECK(t2.thread().id() == sys::thread_id(nullptr));

    t1 = nullptr;
    CHECK_FALSE(t1);
    CHECK_FALSE(t1.joinable());

    sys::once o;
    t1 = sys::managed_thread::ctor([&t1, &o]() -> int
    {
        o.wait();

        CHECK_FALSE(t1.joinable());
        CHECK(t1.join().expect_err() == sys::threading_error::invalid_operation);

        return 42 /* NOLINT(readability-magic-numbers) */;
    }).expect();
    o.call_once([]() -> void { });

    CHECK(t1);
    CHECK(t1.joinable());

    swap(t1, t2);

    CHECK_FALSE(t1);
    CHECK(t1.thread().id() == nullptr);
    CHECK_FALSE(t1.joinable());
    CHECK(t2);
    CHECK(t2.joinable());

    t2.detach();
    CHECK_FALSE(t2.joinable());

    gotComplete = false;
    {
        const sys::managed_thread t = sys::managed_thread::ctor([&]() -> void
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50 /* NOLINT(readability-magic-numbers) */));
            gotComplete = true;
        }).expect();
        // Joinable `sys::thread` joins on destruction.
    }
    CHECK(gotComplete);
}
TEST_CASE("result<managed_thread, void>", "[sys.Threading][result][managed_thread]")
{
    CHECK(sys::result<sys::managed_thread>(sys::managed_thread::ctor([]() -> void { }).expect()).expect().join().expect() == 0);
    CHECK_FALSE(sys::result<sys::managed_thread>(nullptr));
}

TEST_CASE("thread_yield(), thread_current()", "[sys.Threading][thread_yield][thread_current]")
{
    CHECK_NOTHROW(sys::thread_yield()); // Nothing we can really test--but this definitely shouldn't throw!

    CHECK(sys::thread_current().id() != nullptr);
    CHECK(sys::thread_current().id() == sys::thread_current().id());

    sys::thread_id tId = nullptr;
    sys::once o;
    sys::managed_thread t = sys::managed_thread::ctor([&tId, &o]() -> void
    {
        tId = sys::thread_current().id();
        o.call_once([]() -> void { });
    }).expect();
    o.wait();

    // CHECK(tId == t.thread().id()); FIXME(halloimdragon): This doesn't work because Windows gives you a fake handle when retrieving the current thread.
    CHECK(t.join().expect() == 0);
}
TEST_CASE("result<thread, void>", "[sys.Threading][result][thread]")
{
    CHECK(sys::result<sys::thread>(sys::thread_current()).expect().id() == sys::thread_current().id());
    CHECK_FALSE(sys::result<sys::thread>(nullptr));
}

TEST_CASE("thread", "[sys.Threading][thread]")
{
    sys::thread t1 = sys::thread_current();
    sys::thread t2 = std::move(t1);
    t1 = nullptr;

    CHECK_FALSE(t1);
    CHECK(t2);
    CHECK(t1.id() != t2.id());

    t1 = std::move(t2);
    t2 = nullptr;
    swap(t1, t2);

    CHECK_FALSE(t1);
    CHECK(t1.id() == nullptr);
    CHECK(t2);
    CHECK(t2.id() == sys::thread_current().id());
}

TEST_CASE("thread_id", "[sys.Threading][thread_id]")
{
    sys::thread_id id1 = sys::thread_current().id();
    sys::thread_id id2 = std::move(id1);
    id1 = nullptr;

    CHECK_FALSE(id1);
    CHECK(id2);
    CHECK(id1 != id2);

    id1 = std::move(id2);
    id2 = nullptr;
    swap(id1, id2);

    CHECK_FALSE(id1);
    CHECK(id1 == nullptr);
    CHECK(id2);
    CHECK(id2 == sys::thread_current().id());
}
TEST_CASE("result<thread_id, void>", "[sys.Threading][result][thread_id]")
{
    CHECK(sys::result<sys::thread_id>(sys::thread_current().id()).expect() == sys::thread_current().id());
    CHECK_FALSE(sys::result<sys::thread_id>(nullptr));
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
