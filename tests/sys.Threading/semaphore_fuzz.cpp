#include <array>
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
            if (!fail_unlock || std::rand() % 2 == 0)
                return _as(mtx_trylock(&this->mut), internal::threading_error);
            return internal::threading_error::error;
        }
        internal::threading_error lock() noexcept
        {
            if (!fail_unlock || std::rand() % 2 == 0)
                return _as(mtx_lock(&this->mut), internal::threading_error);
            return internal::threading_error::error;
        }
        internal::threading_error unlock() noexcept { return _as(mtx_unlock(&this->mut), internal::threading_error); }
        // NOLINTEND(concurrency-mt-unsafe, misc-predictable-rand)

        friend struct sys::internal::cond_var_handle;
    };

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
            if (!fail_unlock || std::rand() % 2 == 0)
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

#define _libcxxext_internal_mock_sup_mut 1
#define _libcxxext_internal_mock_sup_cv 1
#include <module/sys.Threading>

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("[[fuzz]] binary_semaphore, semaphore, ordinary_semaphore<...>",
                                                                     "[fuzz][sys.Threading][binary_semaphore][semaphore][ordinary_semaphore]", sys::binary_semaphore,
                                                                     sys::semaphore, (sys::ordinary_semaphore<uint_least8_t, _as(1, uint_least8_t)>))
{
    std::srand(_as(std::time({}), unsigned)) /* NOLINT(bugprone-random-generator-seed) */;

    CHECK(rc::check([](const uint64_t) -> void
    {
        sys::internal::fail_unlock = true;
        TestType sem = []() -> TestType
        {
            if constexpr (std::same_as<TestType, sys::semaphore>)
                return TestType(1_uz, unsafe);
            else
                return TestType();
        }();

        auto acqRes = sem.acquire(unsafe);
        while (!acqRes)
        {
            const sys::threading_error err = acqRes.err();
            RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
            acqRes = sem.acquire(unsafe);
        }

        auto relRes = sem.release(unsafe);
        while (!relRes)
        {
            const sys::threading_error err = relRes.err();
            RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
            relRes = sem.release(unsafe);
        }

        auto accRes = sem.access();
        _defer([]() noexcept -> void { sys::internal::fail_unlock = false; });

        while (!accRes)
        {
            const sys::threading_error err = accRes.err();
            RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
            accRes = sem.access();
        }
    }));
}

TEMPLATE_TEST_CASE /* NOLINT(modernize-use-trailing-return-type) */ ("[[fuzz]] semaphore, ordinary_semaphore<...>",
                                                                     "[fuzz][sys.Threading][binary_semaphore][semaphore][ordinary_semaphore]", sys::semaphore,
                                                                     (sys::ordinary_semaphore<uint_least8_t, _as(2, uint_least8_t)>))
{
    std::srand(_as(std::time({}), unsigned)) /* NOLINT(bugprone-random-generator-seed) */;

    CHECK(rc::check([](const uint64_t) -> void
    {
        sys::internal::fail_unlock = true;
        TestType sem = []() -> TestType
        {
            if constexpr (std::same_as<TestType, sys::semaphore>)
                return TestType(2_uz, unsafe);
            else
                return TestType();
        }();

        for (sz i = 0_uz; i < 2_uz; i++ /* NOLINT(bugprone-inc-dec-in-conditions) */)
        {
            auto acqRes = sem.acquire(unsafe);
            while (!acqRes)
            {
                const sys::threading_error err = acqRes.err();
                RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
                acqRes = sem.acquire(unsafe);
            }
        }

        for (sz i = 0_uz; i < 2_uz; i++ /* NOLINT(bugprone-inc-dec-in-conditions) */)
        {
            auto relRes = sem.release(unsafe);
            while (!relRes)
            {
                const sys::threading_error err = relRes.err();
                RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
                relRes = sem.release(unsafe);
            }
        }

        std::array accRes { sem.access(), sem.access() };
        _defer([]() noexcept -> void { sys::internal::fail_unlock = false; });

        for (auto& res : accRes)
        {
            while (!res)
            {
                const sys::threading_error err = res.err();
                RC_ASSERT((err == sys::threading_error::init_failed || err == sys::threading_error::operation_failed));
                res = sem.access();
            }
        }
    }));
}

_nowarn_end_clang();
// NOLINTEND(bugprone-throwing-static-initialization, misc-include-cleaner, readability-function-cognitive-complexity)
