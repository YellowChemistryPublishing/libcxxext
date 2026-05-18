#include <cstdlib>
#include <ctime>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_deprecated();
_nowarn_begin_conv_comp();
_nowarn_begin_unreachable();
_nowarn_begin_one_gcc(_clwarn_gcc_redundant_decls);
_nowarn_begin_one_clang(_clwarn_clang_documentation);
#define NOMINMAX 1 // NOLINT(readability-identifier-naming)

#include <catch2/catch_all.hpp>
#include <rapidcheck.h>
#include <tinycthread.h>

#undef NOMINMAX
#ifdef call_once
#undef call_once
#endif
_nowarn_end_clang();
_nowarn_end_gcc();
_nowarn_end_unreachable();
_nowarn_end_conv_comp();
_nowarn_end_deprecated();

#include <LanguageSupport.h>
#include <sup/ThreadingErrors.h>

void* operator new(size_t size, const std::nothrow_t&) /* NOLINT(readability-inconsistent-declaration-parameter-name) */ noexcept
{
    // NOLINTNEXTLINE(concurrency-mt-unsafe, misc-predictable-rand): I do not care.
    if (std::rand() % 2 == 0)
        return std::malloc(size) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */;
    return nullptr;
}
void operator delete(void* p, const std::nothrow_t&) /* NOLINT(readability-inconsistent-declaration-parameter-name) */ noexcept
{
    std::free(p) /* NOLINT(cppcoreguidelines-owning-memory, hicpp-no-malloc) */;
}

namespace sys::internal
{
    static struct thread_handle thread_current(decltype(unsafe)) noexcept;

    struct thread_handle
    {
    private:
        thrd_t th {};

        thread_handle(thrd_t th, decltype(unsafe)) noexcept : th(th) { }
    public:
        /* NOLINT(hicpp-explicit-conversions) */ thread_handle(std::nullptr_t) { }
        thread_handle(const thread_handle&) noexcept = default;
        thread_handle(thread_handle&&) noexcept = default;
        ~thread_handle() noexcept = default;

        thread_handle& operator=(const thread_handle&) noexcept = default;
        thread_handle& operator=(thread_handle&&) noexcept = default;

        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        friend bool operator==(const thread_handle& a, const thread_handle& b) noexcept { return thrd_equal(a.th, b.th); }

        // NOLINTBEGIN(concurrency-mt-unsafe, misc-predictable-rand): I do not care.
        [[nodiscard]] internal::threading_error create(int (*func)(void*), void* arg, decltype(unsafe)) noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(thrd_create(&this->th, func, arg), internal::threading_error);
            return internal::threading_error::error;
        }
        [[nodiscard]] internal::threading_error join(int* res, decltype(unsafe)) const noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(thrd_join(this->th, res), internal::threading_error);
            return internal::threading_error::error;
        }
        [[nodiscard]] internal::threading_error detach(decltype(unsafe)) const noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(thrd_detach(this->th), internal::threading_error);
            return internal::threading_error::error;
        }
        // NOLINTEND(concurrency-mt-unsafe, misc-predictable-rand)

        friend thread_handle sys::internal::thread_current(decltype(unsafe)) noexcept;
    };

    [[nodiscard]] static inline thread_handle thread_current(decltype(unsafe)) noexcept { return { thrd_current(), unsafe }; }
    static inline void thread_yield(decltype(unsafe)) noexcept { thrd_yield(); }
} // namespace sys::internal

#define _libcxxext_mock_sup_thread 1
#include <module/sys.Threading>

TEST_CASE("[[fuzz]] managed_thread", "[sys.Threading][managed_thread]")
{
    std::srand(_as(std::time({}), unsigned)) /* NOLINT(bugprone-random-generator-seed) */;

    CHECK(rc::check([](const uint64_t) -> void
    {
        auto tRes = sys::managed_thread::ctor([]() -> void { });
        if (!tRes)
        {
            const sys::threading_error err = tRes.err();
            RC_ASSERT(err == sys::threading_error::oom || err == sys::threading_error::init_failed);
            return;
        }
        sys::managed_thread t = tRes.move();

        _defer([&t]() noexcept -> void { t.detach(); });
        auto joinRes = t.join();
        if (!joinRes)
        {
            const sys::threading_error err = joinRes.err();
            RC_ASSERT(err == sys::threading_error::operation_failed);
            return;
        }

        RC_ASSERT(joinRes.expect() == 0);
    }));
}

// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
