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

        sys::once gotWaiting;
        sys::managed_thread t = sys::managed_thread::ctor([&gotWaiting, &cv, &mut]() -> void
        {
            const typename TestType::guard g = mut.lock().expect();
            gotWaiting.call_once([]() -> void { });
            CHECK(cv.wait(mut));
        }).expect();

        gotWaiting.wait();
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

    bool ready = false;
    sys::once gotWaiting;

    sys::managed_thread t = sys::managed_thread::ctor([&ready, &gotWaiting, &cv, &mut]() -> void
    {
        const typename TestType::guard g = mut.lock().expect();
        cv.wait_until(mut, [&ready, &gotWaiting]() -> bool
        {
            gotWaiting.call_once([]() -> void { });
            return ready;
        }).expect();
    }).expect();

    gotWaiting.wait();
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

    bool ready = false;
    sys::once gotWaiting, gotDone;
    i32 gotCount = 0_i32;

    std::array<sys::managed_thread, 4uz> pool { nullptr, nullptr, nullptr, nullptr };
    for (sys::managed_thread& t : pool)
    {
        t = sys::managed_thread::ctor([&ready, &gotWaiting, &gotDone, &gotCount, &mut, &cv]() -> void
        {
            const typename TestType::guard g = mut.lock().expect();
            cv.wait_until(mut, [&ready, &gotWaiting]() -> bool
            {
                gotWaiting.call_once([]() -> void { });
                return ready;
            }).expect();

            ++gotCount;
            gotDone.call_once([]() -> void { });
        }).expect();
    }

    gotWaiting.wait();
    {
        const typename TestType::guard g = mut.lock().expect();
        ready = true;
    }

    CHECK(cv.notify_one());

    gotDone.wait();

    {
        const typename TestType::guard g = mut.lock().expect();
        CHECK(gotCount >= 1_i32);
    }

    CHECK(cv.notify_all());
    for (sys::managed_thread& t : pool)
        CHECK(t.join().expect() == 0);
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("cond_var::notify_all()", "[sys.Threading][cond_var]", sys::mutex, sys::reentrant_mutex)
{
    sys::cond_var cv;
    TestType mut;

    sys::once gotWaiting[4uz], gotDone[4uz];
    bool ready = false;
    i32 gotCount = 0_i32;

    std::array<sys::managed_thread, 4uz> pool { nullptr, nullptr, nullptr, nullptr };
    for (sz i = 0_uz; i < 4_uz; i++)
    {
        pool[i /* NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-constant-array-index) */] =
            sys::managed_thread::ctor([&ready, &gotCount, &gotWaiting = gotWaiting[i /* NOLINT(cppcoreguidelines-pro-bounds-constant-array-index) */],
                                       &gotDone = gotDone[i /* NOLINT(cppcoreguidelines-pro-bounds-constant-array-index) */], &cv, &mut]() -> void
        {
            const typename TestType::guard g = mut.lock().expect();

            cv.wait_until(mut, [&ready, &gotWaiting]() -> bool
            {
                gotWaiting.call_once([]() -> void { });
                return ready;
            }).expect();

            ++gotCount;
            gotDone.call_once([]() -> void { });
        }).expect();
    }

    for (sys::once& w : gotWaiting)
        w.wait();

    {
        const typename TestType::guard g = mut.lock().expect();
        ready = true;
    }
    CHECK(cv.notify_all());

    for (sys::once& w : gotDone)
        w.wait();
    {
        const typename TestType::guard g = mut.lock().expect();
    }

    CHECK(gotCount == 4_i32);
}

_nowarn_end_clang();
// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
