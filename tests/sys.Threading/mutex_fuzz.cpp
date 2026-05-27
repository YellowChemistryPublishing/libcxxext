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
#include <sup/ThreadingErrors.h>

namespace sys::internal
{
    static bool fail_unlock = false; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    struct cond_var_handle;

    /// @internal
    /// @ingroup sys_internal
    struct mutex_handle final // NOLINT(misc-use-internal-linkage)
    {
    private:
        mtx_t mut {};
    public:
        /* NOLINT(hicpp-explicit-conversions) */ mutex_handle(std::nullptr_t) noexcept { }
        mutex_handle(const mutex_handle&) noexcept = delete;
        mutex_handle(mutex_handle&&) noexcept = delete;
        ~mutex_handle() noexcept = default;

        mutex_handle& operator=(const mutex_handle&) noexcept = delete;
        mutex_handle& operator=(mutex_handle&&) noexcept = delete;

        // NOLINTBEGIN(concurrency-mt-unsafe, misc-predictable-rand): I do not care.
        template <bool IsRecursive>
        internal::threading_error create() noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(mtx_init(&this->mut, _as(mtx_plain, unsigned) | _as(IsRecursive ? mtx_recursive : 0, unsigned)), internal::threading_error);
            return internal::threading_error::error;
        }
        void destroy() noexcept { mtx_destroy(&this->mut); }
        internal::threading_error try_lock() noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(mtx_trylock(&this->mut), internal::threading_error);
            return internal::threading_error::error;
        }
        internal::threading_error lock() noexcept
        {
            if (std::rand() % 2 == 0)
                return _as(mtx_lock(&this->mut), internal::threading_error);
            return internal::threading_error::error;
        }
        internal::threading_error unlock() noexcept
        {
            if (!fail_unlock || std::rand() % 2 == 0)
                return _as(mtx_unlock(&this->mut), internal::threading_error);
            return internal::threading_error::error;
        }
        // NOLINTEND(concurrency-mt-unsafe, misc-predictable-rand)

        friend struct sys::internal::cond_var_handle;
    };
} // namespace sys::internal

#define _libcxxext_internal_mock_sup_mut 1
#include <module/sys.Threading>

TEST_CASE("[[fuzz]] mutex", "[fuzz][sys.Threading][mutex]")
{
    std::srand(_as(std::time({}), unsigned)) /* NOLINT(bugprone-random-generator-seed) */;

    CHECK(rc::check([](const uint64_t) -> void
    {
        sys::internal::fail_unlock = false;
        sys::mutex mut;

        (void)mut.lock();
        (void)mut.try_lock();

        sys::internal::fail_unlock = true;

        auto acqRes = mut.acquire(unsafe);
        if (acqRes)
        {
            auto relRes = mut.release(unsafe);
            while (!relRes)
            {
                RC_ASSERT(relRes.err() == sys::threading_error::operation_failed);
                relRes = mut.release(unsafe);
            }
        }
        else
        {
            const sys::threading_error err = acqRes.err();
            RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
        }

        const auto lockRes = mut.lock();
        _defer([]() noexcept -> void { sys::internal::fail_unlock = false; });

        acqRes = mut.try_acquire(unsafe);
        if (acqRes)
        {
            auto relRes = mut.release(unsafe);
            while (!relRes)
            {
                RC_ASSERT(relRes.err() == sys::threading_error::operation_failed);
                relRes = mut.release(unsafe);
            }
        }
        else
        {
            const sys::threading_error err = acqRes.err();
            RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::busy || err == sys::threading_error::operation_failed));
        }
    }));
}

TEST_CASE("[[fuzz]] reentrant_mutex", "[fuzz][sys.Threading][reentrant_mutex]")
{
    std::srand(_as(std::time({}), unsigned)) /* NOLINT(bugprone-random-generator-seed) */;

    CHECK(rc::check([](const uint64_t) -> void
    {
        sys::internal::fail_unlock = false;
        sys::reentrant_mutex mut;

        (void)mut.lock();
        (void)mut.try_lock();

        sys::internal::fail_unlock = true;

        auto acqRes = mut.acquire(unsafe);
        if (acqRes)
        {
            auto relRes = mut.release(unsafe);
            while (!relRes)
            {
                RC_ASSERT(relRes.err() == sys::threading_error::operation_failed);
                relRes = mut.release(unsafe);
            }
        }
        else
        {
            const sys::threading_error err = acqRes.err();
            RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
        }

        const auto lockRes = mut.lock();
        _defer([]() noexcept -> void { sys::internal::fail_unlock = false; });

        acqRes = mut.try_acquire(unsafe);
        if (acqRes)
        {
            auto relRes = mut.release(unsafe);
            while (!relRes)
            {
                RC_ASSERT(relRes.err() == sys::threading_error::operation_failed);
                relRes = mut.release(unsafe);
            }
        }
        else
        {
            const sys::threading_error err = acqRes.err();
            RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
        }
    }));
}

_nowarn_end_clang();
// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
