#include <atomic>
#include <stdexcept>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);

#include <catch2/catch_all.hpp>

_nowarn_end_gcc();

#include <module/sys>
#include <module/sys.Threading>

TEST_CASE("once::wait()", "[sys.Threading][once]")
{
    sys::once o;
    CHECK_FALSE(o.is_completed());

    sys::managed_thread t = sys::managed_thread::ctor([&o]() -> void
    {
        o.wait();
        CHECK(o.is_completed());
    }).expect();
    o.call_once([]() -> void { });

    CHECK(o.is_completed());

    o.wait();

    CHECK(o.is_completed());
    CHECK(t.join().expect() == 0);
}

TEST_CASE("once::call_once(-> !std::same_as<result<...>>)", "[sys.Threading][once]")
{
    std::atomic<size_t> gotCallCount = 0;
    sys::once o;
    std::array<sys::managed_thread, 24uz /* NOLINT(readability-magic-numbers) */> pool { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                                                         nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                                                         nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    for (sys::managed_thread& t : pool)
    {
        t = sys::managed_thread::ctor([&gotCallCount, &o]() -> void
        {
            CHECK_THROWS_AS(o.call_once(
                                [&gotCallCount](const sz add) -> void
            {
                gotCallCount += add;
                throw std::runtime_error("oops");
            }, 1_uz),
                            std::runtime_error);
            CHECK(!o.is_completed());
        }).expect();
    }

    for (sys::managed_thread& t : pool)
        CHECK(t.join().expect() == 0);
    CHECK(gotCallCount == 24uz);

    for (sys::managed_thread& t : pool)
    {
        t = sys::managed_thread::ctor([&gotCallCount, &o]() -> void
        {
            o.call_once([&gotCallCount]() -> void { ++gotCallCount; });
            CHECK(o.is_completed());
        }).expect();
    }

    for (sys::managed_thread& t : pool)
        CHECK(t.join().expect() == 0);
    CHECK(gotCallCount == 25uz);
}

TEST_CASE("once::call_once(-> std::same_as<result<...>>)", "[sys.Threading][once]")
{
    std::atomic<size_t> gotCallCount = 0;
    sys::once o;
    std::array<sys::managed_thread, 24uz /* NOLINT(readability-magic-numbers) */> pool { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                                                         nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                                                         nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    for (sys::managed_thread& t : pool)
    {
        t = sys::managed_thread::ctor([&gotCallCount, &o]() -> void
        {
            CHECK(o.call_once(
                       [&gotCallCount](const i8 ret) -> sys::result<void, i8>
            {
                gotCallCount += 1uz;
                return ret;
            }, 64_i8)
                      .expect_err() == 64_i8);
            CHECK(!o.is_completed());
        }).expect();
    }

    for (sys::managed_thread& t : pool)
        CHECK(t.join().expect() == 0);
    CHECK(gotCallCount == 24uz);

    _nowarn_begin_one_msvc(_clwarn_msvc_unreachable_code);
    CHECK(sys::managed_thread::ctor(
              [&gotCallCount, &o]() -> void
    {
        CHECK_THROWS_AS(o.call_once(
                             [&gotCallCount](const sz add, const i16 ret) -> sys::result<void, i16>
        {
            gotCallCount += add;
            throw std::runtime_error("oops");
            return ret;
        }, 1_uz, 32_i16)
                                .expect_err() == 32_i16,
                        std::runtime_error);
        CHECK(!o.is_completed());
    })
              .expect()
              .join()
              .expect() == 0);
    CHECK(gotCallCount == 25uz);
    _nowarn_end_msvc();

    for (sys::managed_thread& t : pool)
    {
        t = sys::managed_thread::ctor([&gotCallCount, &o]() -> void
        {
            (void)o.call_once([]() -> sys::result<void> { return nullptr; });

            CHECK(o.call_once([&gotCallCount]() -> sys::result<void>
            {
                gotCallCount += 1uz;
                return {};
            }));
            CHECK(o.is_completed());
        }).expect();
    }

    for (sys::managed_thread& t : pool)
        CHECK(t.join().expect() == 0);
    CHECK(gotCallCount == 26uz);
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
