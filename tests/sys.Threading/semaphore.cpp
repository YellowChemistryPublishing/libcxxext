#include <SemaphoreEx.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Threading>

static_assert(sys::INothrowDefaultConstructible<sys::binary_semaphore>);
static_assert(!sys::INothrowDefaultConstructible<sys::semaphore>);

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Semaphore acquire/release works.", "[sys.Threading][semaphore]")
{
    sys::semaphore sem(1_uz, unsafe);

    sem.acquire(unsafe).expect();
    sem.release(unsafe).expect();
    sem.acquire(unsafe).expect();
    sem.release(unsafe).expect();
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Semaphore constructors are available where expected.", "[sys.Threading][semaphore]")
{
    sys::semaphore sem(2_uz, unsafe);
    sem.acquire(unsafe).expect();
    sem.acquire(unsafe).expect();
    sem.release(unsafe).expect();
    sem.release(unsafe).expect();
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Semaphore access guard releases on destruction.", "[sys.Threading][semaphore]")
{
    sys::semaphore sem(1_uz, unsafe);
    std::atomic_flag workerStarted = ATOMIC_FLAG_INIT;
    std::atomic_flag workerAcquired = ATOMIC_FLAG_INIT;
    sys::managed_thread worker = nullptr;

    {
        const sys::semaphore::guard guard = sem.access().expect();

        worker = sys::managed_thread::ctor([&]() -> void
        {
            workerStarted.test_and_set(std::memory_order_release);
            const sys::semaphore::guard g = sem.access().expect();
            workerAcquired.test_and_set(std::memory_order_release);
        }).expect();

        while (!workerStarted.test(std::memory_order_acquire))
            sys::thread_yield();

        std::this_thread::sleep_for(std::chrono::milliseconds(20 /* NOLINT(readability-magic-numbers) */));
        CHECK_FALSE(workerAcquired.test(std::memory_order_acquire));
    }

    while (!workerAcquired.test(std::memory_order_acquire))
        sys::thread_yield();

    const sys::integer<int> tret = worker.join().expect();
    CHECK(tret == 0);
    CHECK(workerAcquired.test(std::memory_order_acquire));
}

TEST_CASE("Semaphore guard move ctor/assign preserve single release.", "[sys.Threading][semaphore]")
{
    sys::binary_semaphore sem;

    sys::binary_semaphore::guard g1 = sem.access().expect();
    sys::binary_semaphore::guard g2 = std::move(g1);
    sys::binary_semaphore::guard g3;
    g3 = std::move(g2);

    std::atomic_flag workerAcquired = ATOMIC_FLAG_INIT;
    sys::managed_thread worker = sys::managed_thread::ctor([&]() -> void
    {
        const sys::binary_semaphore::guard g = sem.access().expect();
        workerAcquired.test_and_set(std::memory_order_release);
    }).expect();

    std::this_thread::sleep_for(std::chrono::milliseconds(20 /* NOLINT(readability-magic-numbers) */));
    CHECK_FALSE(workerAcquired.test(std::memory_order_acquire));

    g3 = sys::binary_semaphore::guard();
    while (!workerAcquired.test(std::memory_order_acquire))
        sys::thread_yield();

    const sys::integer<int> tret = worker.join().expect();
    CHECK(tret == 0);
}

TEST_CASE("Semaphore limits concurrent accessors under contention.", "[sys.Threading][semaphore]")
{
    constexpr i32 maxPermits = 3;
    constexpr i32 numThreads = 12;

    sys::semaphore sem(_as(maxPermits, sz), unsafe);
    std::atomic<int> inFlight = 0;
    std::atomic<int> maxInFlight = 0;
    std::vector<sys::managed_thread> threads;

    for (i32 i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(sys::managed_thread::ctor([&]() -> void
        {
            const sys::semaphore::guard guard = sem.access().expect();
            const int now = ++inFlight;

            int observed = maxInFlight.load(std::memory_order_relaxed);
            while (observed < now && !maxInFlight.compare_exchange_weak(observed, now, std::memory_order_relaxed))
            { }

            std::this_thread::sleep_for(std::chrono::milliseconds(2 /* NOLINT(readability-magic-numbers) */));
            --inFlight;
        }).expect());
    }

    threads.clear();
    CHECK(maxInFlight.load() <= maxPermits);
}

TEST_CASE("Semaphore release reports overflow and preserves state.", "[sys.Threading][semaphore]")
{
    sys::ordinary_semaphore<uint_least8_t> sem(*u8::highest(), unsafe);

    CHECK(sem.release(unsafe).expect_err() == sys::threading_error::overflow);

    // Ensure failed release did not corrupt permits.
    const sys::ordinary_semaphore<uint_least8_t>::guard g1 = sem.access().expect();
    const sys::ordinary_semaphore<uint_least8_t>::guard g2 = sem.access().expect();
}

TEST_CASE("Binary semaphore default ctor starts with one permit.", "[sys.Threading][semaphore]")
{
    sys::binary_semaphore sem;
    std::atomic_flag secondAcquired = ATOMIC_FLAG_INIT;
    sys::binary_semaphore::guard first = sem.access().expect();

    sys::managed_thread worker = sys::managed_thread::ctor([&]() -> void
    {
        const sys::binary_semaphore::guard second = sem.access().expect();
        secondAcquired.test_and_set(std::memory_order_release);
    }).expect();

    std::this_thread::sleep_for(std::chrono::milliseconds(20 /* NOLINT(readability-magic-numbers) */));
    CHECK_FALSE(secondAcquired.test(std::memory_order_acquire));
    first = sys::binary_semaphore::guard();

    while (!secondAcquired.test(std::memory_order_acquire))
        sys::thread_yield();

    const sys::integer<int> tret = worker.join().expect();
    CHECK(tret == 0);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner)
