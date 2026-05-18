// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Threading>

TEST_CASE("binary_semaphore::binary_semaphore(), semaphore::semaphore(...), ordinary_semaphore<...>::ordinary_semaphore(...)",
          "[sys.Threading][binary_semaphore][semaphore][ordinary_semaphore]")
{
    sys::binary_semaphore();                                         // NOLINT(bugprone-unused-raii)
    sys::semaphore(0_uz, unsafe);                                    // NOLINT(bugprone-unused-raii)
    sys::ordinary_semaphore<uint_least8_t, _as(3, uint_least8_t)>(); // NOLINT(bugprone-unused-raii)
    sys::ordinary_semaphore<uint_least8_t>(2_u8, unsafe);            // NOLINT(bugprone-unused-raii)
}

TEST_CASE("binary_semaphore::acquire(...), binary_semaphore::release(...), semaphore::acquire(...), semaphore::release(...),"
          "ordinary_semaphore<...>::acquire(...), ordinary_semaphore<...>::release(...)",
          "[sys.Threading][binary_semaphore][semaphore][ordinary_semaphore]")
{
    {
        sys::semaphore sem(1_uz, unsafe);
        sem.acquire(unsafe).expect();
        sem.release(unsafe).expect();
        sem.acquire(unsafe).expect();
        sem.release(unsafe).expect();
    }

    {
        sys::semaphore sem(2_uz, unsafe);
        sem.acquire(unsafe).expect();
        sem.acquire(unsafe).expect();
        sem.release(unsafe).expect();
        sem.release(unsafe).expect();
    }

    {
        sys::binary_semaphore sem;
        sem.acquire(unsafe).expect();
        sem.release(unsafe).expect();
    }

    {
        sys::ordinary_semaphore<uint_least8_t> sem(u8::highest(), unsafe);

        CHECK(sem.release(unsafe).expect_err() == sys::threading_error::overflow);

        const sys::ordinary_semaphore<uint_least8_t>::guard g1 = sem.access().expect();
        const sys::ordinary_semaphore<uint_least8_t>::guard g2 = sem.access().expect();
    }
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("binary_semaphore::guard, semaphore::guard, ordinary_semaphore<...>::guard",
                                                                     "[sys.Threading][binary_semaphore][semaphore][ordinary_semaphore]", sys::binary_semaphore, sys::semaphore,
                                                                     (sys::ordinary_semaphore<uint_least8_t, _as(4, uint_least8_t)>))
{
    TestType sem = []() -> TestType
    {
        if constexpr (std::same_as<TestType, sys::semaphore>)
            return TestType(1_uz, unsafe);
        else
            return TestType();
    }();

    typename TestType::guard guard1 = sem.access().expect();
    typename TestType::guard guard2 = std::move(guard1);
    guard1 = std::move(guard2);
    guard2 = typename TestType::guard();

    CHECK(guard1);
    CHECK_FALSE(guard2);
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("binary_semaphore::access(), semaphore::access(), ordinary_semaphore<...>::access()",
                                                                     "[sys.Threading][binary_semaphore][semaphore][ordinary_semaphore]", sys::binary_semaphore, sys::semaphore,
                                                                     (sys::ordinary_semaphore<uint_least8_t, _as(1, uint_least8_t)>))
{
    TestType sem = []() -> TestType
    {
        if constexpr (std::same_as<TestType, sys::semaphore>)
            return TestType(1_uz, unsafe);
        else
            return TestType();
    }();
    sys::once gotReady, gotAcquired;
    sys::managed_thread t = nullptr;

    {
        const typename TestType::guard guard = sem.access().expect();
        t = sys::managed_thread::ctor([&gotAcquired, &gotReady, &sem]() -> void
        {
            gotReady.call_once([]() -> void { });
            const typename TestType::guard g = sem.access().expect();
            gotAcquired.call_once([]() -> void { });
        }).expect();

        gotReady.wait();
        CHECK_FALSE(gotAcquired.is_completed());
    }

    gotAcquired.wait();
    CHECK(t.join().expect() == 0);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
