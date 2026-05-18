// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_clang(_clwarn_clang_consumed);
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Threading>

static_assert(sys::INothrowDefaultConstructible<sys::cond_var>);
static_assert(sys::INothrowDestructible<sys::cond_var>);

TEST_CASE("cond_var::cond_var()", "[sys.Threading][cond_var]")
{
    sys::cond_var(); // NOLINT(bugprone-unused-raii)

    sys::cond_var cv;
    CHECK(cv.notify_one());
    CHECK(cv.notify_all());
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("cond_var::wait(...)", "[sys.Threading][cond_var]", sys::mutex, sys::reentrant_mutex)
{
    {
        sys::cond_var cv;
        TestType mut;

        sys::once waiting;
        sys::managed_thread t = sys::managed_thread::ctor([&waiting, &cv, &mut]() -> void
        {
            const typename TestType::guard g = mut.lock().expect();
            waiting.call_once([]() -> void { });
            CHECK(cv.wait(mut));
        }).expect();

        waiting.wait();
        {
            const typename TestType::guard g = mut.lock().expect();
        }

        CHECK(cv.notify_one());
        CHECK(t.join().expect() == 0);
    }
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("cond_var::wait_until(...)", "[sys.Threading][cond_var]", sys::mutex, sys::reentrant_mutex)
{
    sys::cond_var cv;
    TestType mut;

    sys::once waiting;
    bool ready = false;

    sys::managed_thread t = sys::managed_thread::ctor([&ready, &waiting, &cv, &mut]() -> void
    {
        const typename TestType::guard g = mut.lock().expect();
        cv.wait_until(mut, [&ready, &waiting]() -> bool
        {
            waiting.call_once([]() -> void { });
            return ready;
        }).expect();
    }).expect();

    waiting.wait();
    {
        const typename TestType::guard g = mut.lock().expect();
        ready = true;
    }

    CHECK(cv.notify_one());
    CHECK(t.join().expect() == 0);
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("cond_var::notify_one()", "[sys.Threading][cond_var]", sys::mutex, sys::reentrant_mutex)
{
    sys::cond_var cv;
    TestType mut;

    sys::once waitingReady[4uz], waitingDone[4uz];
    sz threshold = 0_uz;
    i32 gotCount = 0_i32;

    std::array<sys::managed_thread, 4uz> pool { nullptr, nullptr, nullptr, nullptr };
    for (sz i = 0_uz; i < 4_uz; i++)
    {
        pool[i /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-constant-array-index) */] =
            sys::managed_thread::ctor([i, &threshold, &waitingReady = waitingReady[i /* NOLINT(cppcoreguidelines-pro-bounds-constant-array-index) */],
                                       &waitingDone = waitingDone[i /* NOLINT(cppcoreguidelines-pro-bounds-constant-array-index) */], &gotCount, &mut, &cv]() -> void
        {
            const typename TestType::guard g = mut.lock().expect();

            cv.wait_until(mut, [&i, &threshold, &waitingReady]() -> bool
            {
                waitingReady.call_once([]() -> void { });
                return i < threshold;
            }).expect();

            ++gotCount;
            waitingDone.call_once([]() -> void { });
        }).expect();
    }

    {
        const typename TestType::guard g = mut.lock().expect();
        threshold = 1_uz;
    }

    waitingReady[0uz].wait();
    CHECK(cv.notify_one());
    waitingDone[0uz].wait();

    {
        const typename TestType::guard g = mut.lock().expect();
        CHECK(gotCount == 1_i32);
        threshold = 2_uz;
    }

    waitingReady[1uz].wait();
    CHECK(cv.notify_one());
    waitingDone[1uz].wait();

    {
        const typename TestType::guard g = mut.lock().expect();
        CHECK(gotCount == 2_i32);
        threshold = 3_uz;
    }

    waitingReady[2uz].wait();
    CHECK(cv.notify_one());
    waitingDone[2uz].wait();

    {
        const typename TestType::guard g = mut.lock().expect();
        CHECK(gotCount == 3_i32);
        threshold = 4_uz;
    }

    waitingReady[3uz].wait();
    CHECK(cv.notify_one());
    waitingDone[3uz].wait();

    {
        const typename TestType::guard g = mut.lock().expect();
        CHECK(gotCount == 4_i32);
    }

    for (sys::managed_thread& t : pool)
        CHECK(t.join().expect() == 0);
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("cond_var::notify_all()", "[sys.Threading][cond_var]", sys::mutex, sys::reentrant_mutex)
{
    sys::cond_var cv;
    TestType mut;

    sys::once waitingReady[4uz], waitingDone[4uz];
    bool ready = false;
    i32 gotCount = 0_i32;

    std::array<sys::managed_thread, 4uz> pool { nullptr, nullptr, nullptr, nullptr };
    for (sz i = 0_uz; i < 4_uz; i++)
    {
        pool[i /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-constant-array-index) */] =
            sys::managed_thread::ctor([&ready, &gotCount, &waitingReady = waitingReady[i /* NOLINT(cppcoreguidelines-pro-bounds-constant-array-index) */],
                                       &waitingDone = waitingDone[i /* NOLINT(cppcoreguidelines-pro-bounds-constant-array-index) */], &cv, &mut]() -> void
        {
            const typename TestType::guard g = mut.lock().expect();

            cv.wait_until(mut, [&ready, &waitingReady]() -> bool
            {
                waitingReady.call_once([]() -> void { });
                return ready;
            }).expect();

            ++gotCount;
            waitingDone.call_once([]() -> void { });
        }).expect();
    }

    for (sys::once& w : waitingReady)
        w.wait();

    {
        const typename TestType::guard g = mut.lock().expect();
        ready = true;
    }
    CHECK(cv.notify_all());

    for (sys::once& w : waitingDone)
        w.wait();
    {
        const typename TestType::guard g = mut.lock().expect();
    }

    CHECK(gotCount == 4_i32);
}

_nowarn_end_clang();
// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
