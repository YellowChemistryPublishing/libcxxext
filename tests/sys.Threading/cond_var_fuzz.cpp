#include <cstdlib>
#include <ctime>

// NOLINTBEGIN(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
#include <CompilerWarnings.h>
_nowarn_begin_one_clang(_clwarn_clang_consumed);
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
#include <sup/MutexHandle.h>
#include <sup/ThreadingErrors.h>

namespace sys::internal
{
    struct cond_var_handle final // NOLINT(misc-use-internal-linkage)
    {
    private:
        cnd_t cond {};
    public:
        /* NOLINT(hicpp-explicit-conversions) */ cond_var_handle(std::nullptr_t) noexcept { }
        cond_var_handle(const cond_var_handle&) noexcept = delete;
        cond_var_handle(cond_var_handle&&) noexcept = delete;
        ~cond_var_handle() noexcept = default;

        cond_var_handle& operator=(const cond_var_handle&) noexcept = delete;
        cond_var_handle& operator=(cond_var_handle&&) noexcept = delete;

        // NOLINTBEGIN(concurrency-mt-unsafe, misc-predictable-rand): I do not care.
        internal::threading_error create() noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(cnd_init(&this->cond), internal::threading_error);
            return internal::threading_error::error;
        }
        void destroy() noexcept { cnd_destroy(&this->cond); }
        internal::threading_error signal() noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(cnd_signal(&this->cond), internal::threading_error);
            return internal::threading_error::error;
        }
        internal::threading_error broadcast() noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(cnd_broadcast(&this->cond), internal::threading_error);
            return internal::threading_error::error;
        }
        internal::threading_error wait(internal::mutex_handle& mut) noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(cnd_wait(&this->cond, &mut.mut), internal::threading_error);
            return internal::threading_error::error;
        }
        // NOLINTEND(concurrency-mt-unsafe, misc-predictable-rand)
    };
} // namespace sys::internal

#define _libcxxext_mock_sup_cv 1
#include <module/sys.Threading>

TEST_CASE("[[fuzz]] cond_var", "[sys.Threading][cond_var]")
{
    std::srand(_as(std::time({}), unsigned)) /* NOLINT(bugprone-random-generator-seed) */;

    CHECK(rc::check([](const uint64_t) -> void
    {
        sys::cond_var cv;

        if (auto res = cv.notify_one(); !res)
        {
            const sys::threading_error err = res.err();
            RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
        }

        if (auto res = cv.notify_all(); !res)
        {
            const sys::threading_error err = res.err();
            RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
        }

        sys::once signalDone;
        sys::managed_thread t = sys::managed_thread::ctor([&signalDone, &cv]() -> void
        {
            while (!signalDone.is_completed())
            {
                if (auto res = cv.notify_one(); !res)
                {
                    const sys::threading_error err = res.err();
                    RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
                }
            }
        }).expect();

        {
            sys::mutex mut;
            const sys::mutex::guard g = mut.lock().expect();

            if (auto res = cv.wait(mut); !res)
            {
                const sys::threading_error err = res.err();
                RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
            }

            if (auto res = cv.wait_until(mut, []() -> bool { return std::rand() /* NOLINT(concurrency-mt-unsafe, misc-predictable-rand): I do not care. */ % 2 == 0; }); !res)
            {
                const sys::threading_error err = res.err();
                RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
            }
        }

        signalDone.call_once([]() -> void { });
        CHECK(t.join().expect() == 0);
    }));
}

_nowarn_end_clang();
// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
