#pragma once

/// @file

#define NOMINMAX 1 // NOLINT(readability-identifier-naming)
#include <tinycthread.h>
#include <utility>
#undef NOMINMAX
#ifdef call_once
#undef call_once
#endif

#include <LanguageSupport.h>
#include <Once.h>
#include <Result.h>
#include <ThreadingErrors.h>

namespace sys
{
    class cond_var;

    template <bool IsRecursive>
    class ordinary_mutex final
    {
        mtx_t mut {};
        once o;
    public:
        struct guard final
        {
        private:
            mtx_t* mut = nullptr;
        public:
            /// @warning `unsafe` because `mut` must be locked.
            explicit guard(mtx_t& mut, unsafe) noexcept : mut(&mut) { }
            guard(const guard&) = delete;
            guard(guard&& other) noexcept : mut(std::exchange(other.mut, nullptr)) { }
            ~guard() noexcept { _contract_assert(!this->mut || mtx_unlock(this->mut) == thrd_success, "If this happens we're genuinely cooked."); }

            guard& operator=(const guard&) = delete;
            guard& operator=(guard&& other) noexcept
            {
                _contract_assert(!this->mut || mtx_unlock(this->mut) == thrd_success, "If this happens we're genuinely cooked.");
                this->mut = std::exchange(other.mut, nullptr);
                return *this;
            }
        };
    private:
        template <int (*DoLock)(mtx_t*)>
        result<guard, threading_error> lock(unsafe) noexcept
        {
            _retif(threading_error::init_failed,
                   !this->o.call_once([&]() noexcept -> sys::result<void>
            {
                _retif(nullptr, mtx_init(&this->mut, _as(unsigned, mtx_plain) | _as(unsigned, mtx_timed) | _as(unsigned, IsRecursive ? mtx_recursive : 0)) != thrd_success);
                return {};
            }));

            _retif(threading_error::operation_failed, DoLock(&this->mut) != thrd_success);
            return guard(this->mut, unsafe());
        }
    public:
        ordinary_mutex() noexcept = default;
        ordinary_mutex(const ordinary_mutex&) noexcept = delete;
        ordinary_mutex(ordinary_mutex&&) noexcept = delete;
        ~ordinary_mutex() noexcept
        {
            if (this->o.is_completed()) [[likely]]
                mtx_destroy(&this->mut);
        }

        ordinary_mutex& operator=(const ordinary_mutex&) noexcept = delete;
        ordinary_mutex& operator=(ordinary_mutex&&) noexcept = delete;

        /// @brief Lock and obtain a lock guard for this mutex.
        result<guard, threading_error> lock() noexcept { return this->lock<&mtx_lock>(unsafe()); }
        /// @brief Try to lock this mutex and obtain a lock guard for it, if possible.
        result<guard, threading_error> try_lock() noexcept { return this->lock<&mtx_trylock>(unsafe()); }
        // TODO(halloimdragon): `try_lock_for(...)`.

        friend class sys::cond_var;
    };

    using mutex = ordinary_mutex<false>;
    using reentrant_mutex = ordinary_mutex<true>;
} // namespace sys

#undef timespec // NOLINT(misc-include-cleaner)
