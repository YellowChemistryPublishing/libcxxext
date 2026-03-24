#pragma once

/// @file

#include <atomic>
#include <tinycthread.h>
#include <utility>

#include <LanguageSupport.h>
#include <Result.h>
#include <ThreadEx.h>
#include <ThreadingErrors.h>

namespace sys
{
    class mutex final
    {
        mtx_t mut {};
        std::atomic_flag initialized;
        std::atomic_flag busy;
    public:
        struct guard final
        {
        private:
            mtx_t* mut = nullptr;
        public:
            /// @warning `unsafe` because `mut` must be locked.
            explicit guard(mtx_t& mut, unsafe) noexcept : mut(&mut) { }
            guard(const guard&) = delete;
            guard(guard&& other) noexcept { std::swap(this->mut, other.mut); }
            ~guard() noexcept { _contract_assert(!this->mut || mtx_unlock(this->mut) == thrd_success, "If this happens we're genuinely cooked."); }

            guard& operator=(const guard&) = delete;
            /// @note Abusable with no real benefit.
            guard& operator=(guard&& other) noexcept = delete;
        };
    private:
        result<guard, threading_error> lock(int (*do_lock)(mtx_t*), unsafe) noexcept
        {
            if (this->initialized.test(std::memory_order_acquire)) [[likely]]
                goto AlreadyInitialized;

            while (this->busy.test_and_set(std::memory_order_acquire))
                thread_yield();

            if (this->initialized.test(std::memory_order_acquire)) [[likely]]
            {
                this->busy.clear(std::memory_order_release);
                goto AlreadyInitialized;
            }

            if (mtx_init(&this->mut, mtx_plain) != thrd_success) [[unlikely]]
            {
                this->busy.clear(std::memory_order_release);
                return threading_error::init_failed;
            }

            this->initialized.test_and_set(std::memory_order_release);
            this->busy.clear(std::memory_order_release);
        AlreadyInitialized:
            _retif(threading_error::operation_failed, do_lock(&this->mut) != thrd_success);
            return guard(this->mut, unsafe());
        }
    public:
        mutex() noexcept = default;
        mutex(const mutex&) = delete;
        mutex(mutex&&) noexcept = delete;
        ~mutex() noexcept
        {
            if (this->initialized.test(std::memory_order_acquire))
                mtx_destroy(&this->mut);
        }

        mutex& operator=(const mutex&) = delete;
        mutex& operator=(mutex&&) noexcept = delete;

        /// @brief Lock and obtain a lock guard for this mutex.
        result<guard, threading_error> lock() noexcept { return this->lock(&mtx_lock, unsafe()); }
        /// @brief Try to lock this mutex and obtain a lock guard for it, if possible.
        result<guard, threading_error> try_lock() noexcept { return this->lock(&mtx_trylock, unsafe()); }
    };
} // namespace sys

#undef timespec // NOLINT(misc-include-cleaner)
