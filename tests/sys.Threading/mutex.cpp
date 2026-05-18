// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_clang(_clwarn_clang_consumed);
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Threading>

TEST_CASE("mutex::mutex(), reentrant_mutex::reentrant_mutex()", "[sys.Threading][mutex][reentrant_mutex]")
{
    sys::mutex(); // NOLINT(bugprone-unused-raii)

    {
        sys::mutex mut;
        const sys::mutex::guard guard = mut.lock().expect();
    }

    sys::reentrant_mutex(); // NOLINT(bugprone-unused-raii)

    {
        sys::reentrant_mutex rmut;
        const sys::reentrant_mutex::guard rguard1 = rmut.lock().expect();
        const sys::reentrant_mutex::guard rguard2 = rmut.lock().expect();
    }
}

TEST_CASE("mutex::acquire(...), mutex::try_acquire(...), mutex::release(...), reentrant_mutex::acquire(...), reentrant_mutex::try_acquire(...), reentrant_mutex::release(...)",
          "[sys.Threading][mutex][reentrant_mutex]")
{
    sys::mutex mut;

    CHECK(mut.acquire(unsafe));
    CHECK(mut.try_acquire(unsafe).expect_err() == sys::threading_error::busy);
    CHECK(mut.release(unsafe));

    CHECK(mut.try_acquire(unsafe));
    CHECK(mut.release(unsafe));

    sys::reentrant_mutex rmut;

    CHECK(rmut.acquire(unsafe));
    CHECK(rmut.try_acquire(unsafe));
    CHECK(rmut.release(unsafe));
    CHECK(rmut.release(unsafe));

    CHECK(rmut.try_acquire(unsafe));
    CHECK(rmut.release(unsafe));
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("mutex::guard, reentrant_mutex::guard", "[sys.Threading][mutex][reentrant_mutex]", sys::mutex,
                                                                     sys::reentrant_mutex)
{
    TestType mut;
    typename TestType::guard guard1 = mut.lock().expect();
    typename TestType::guard guard2 = std::move(guard1);
    guard1 = std::move(guard2);
    guard2 = typename TestType::guard();

    CHECK(guard1);
    CHECK_FALSE(guard2);
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("mutex::lock(...), mutex::try_lock(...), reentrant_mutex::lock(...), reentrant_mutex::try_lock(...)",
                                                                     "[sys.Threading][mutex][reentrant_mutex]", sys::mutex, sys::reentrant_mutex)
{
    TestType mut;

    {
        const typename TestType::guard guard = mut.lock().expect();
        if constexpr (std::same_as<TestType, sys::mutex>)
            CHECK(mut.try_lock().expect_err() == sys::threading_error::busy);
        else
            CHECK(mut.try_lock());
    }

    u8 gotCount = 0_u8;
    std::array<sys::managed_thread, 24uz /* NOLINT(readability-magic-numbers) */> pool { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                                                         nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                                                         nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    for (sys::managed_thread& t : pool)
    {
        t = sys::managed_thread::ctor([&gotCount, &mut]() -> void
        {
            for (sz i = 0_uz; i < 4_uz /* NOLINT(readability-magic-numbers) */; i++)
            {
                const typename TestType::guard g = mut.lock().expect();
                gotCount += 1_u8;
            }
        }).expect();
    }

    for (sys::managed_thread& t : pool)
        CHECK(t.join().expect() == 0);
    CHECK(gotCount == 96_u8);
}

_nowarn_end_clang();
// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
