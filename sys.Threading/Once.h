#pragma once

/// @file

#include <atomic>
#include <utility>

#include <Destructor.h>
#include <Result.h>
#include <ThreadEx.h>
#include <meta/Type.h>

namespace sys
{
    class once final
    {
        std::atomic_flag initialized;
        std::atomic_flag busy;

        bool pre_call() noexcept
        {
            if (this->is_completed())
                return true;

            while (this->busy.test_and_set(std::memory_order_acquire))
                thread_yield();

            if (this->is_completed())
            {
                this->busy.clear(std::memory_order_release);
                return true;
            }

            return false;
        }
    public:
        once() noexcept = default;
        once(const once&) noexcept = delete;
        once(once&&) noexcept = delete;
        ~once() noexcept = default;

        once& operator=(const once&) noexcept = delete;
        once& operator=(once&&) noexcept = delete;

        /// @brief Checks if the function has been successfully executed.
        [[nodiscard]] bool is_completed() const noexcept { return this->initialized.test(std::memory_order_acquire); }

        /// @brief Waits until the function has been fully executed, or returns immediately if already done.
        void wait() noexcept
        {
            if (this->is_completed()) [[likely]]
                return;

            while (this->busy.test_and_set(std::memory_order_acquire))
                thread_yield();

            this->busy.clear(std::memory_order_release);
        }

        /// @brief Calls `func` with `args...`, ensuring only one functor passed to `.call_once(...)` is ever run.
        /// @note If an exception is thrown, this `sys::once` can be called again.
        template <typename Func, typename... Args>
        void call_once(Func&& func, Args&&... args) noexcept(noexcept(func(std::forward<Args>(args)...)))
        requires (!meta::type<decltype(func(args...))>::template is_from<result>())
        {
            if (this->pre_call())
                return;

            const sys::destructor releaseBusy = [&] noexcept { this->busy.clear(std::memory_order_release); };
            std::forward<Func>(func)(std::forward<Args>(args)...);

            this->initialized.test_and_set(std::memory_order_release);
        }

        /// @brief Calls `func` with `args...`, ensuring only one functor passed to `.call_once(...)` is ever run.
        /// @return If `func(args...)` produces a `sys::result<T, Err>`, on failure, an `Err` is produced, otherwise nothing.
        /// @note If an exception is thrown, this `sys::once` can be called again.
        template <typename Func, typename... Args>
        sys::result<void, decltype(std::declval<Func>()(std::declval<Args&&>()...).err())> call_once(Func&& func,
                                                                                                     Args&&... args) noexcept(noexcept(func(std::forward<Args>(args)...)))
        requires (meta::type<decltype(func(args...))>::template is_from<result>())
        {
            if (this->pre_call())
                return {};

            const sys::destructor releaseBusy = [&] noexcept { this->busy.clear(std::memory_order_release); };
            if (auto res = std::forward<Func>(func)(std::forward<Args>(args)...); !res)
                return res.err();

            this->initialized.test_and_set(std::memory_order_release);
            return {};
        }
    };
} // namespace sys
