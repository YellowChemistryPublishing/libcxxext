#pragma once

/// @file

#include <atomic>
#include <concepts>
#include <type_traits>

#include <Destructor.h>
#include <Result.h>
#include <ThreadEx.h>
#include <meta/InterfaceRequirements.h>
#include <meta/NamedRequirements.h>
#include <meta/Type.h>

namespace sys
{
    /// @ingroup sys_threading
    /// @brief A thread-safe, one-time initialization primitive.
    /// @details
    /// This class ensures that for any `sys::once`, only one function passed to `.call_once(...)` is ever run, even when called by multiple threads.
    ///
    /// Implements `sys::INothrowDefaultConstructible`, `sys::INothrowDestructible`.
    /// @see
    /// For more information on `sys::once`, see
    /// [Rust Docs](https://doc.rust-lang.org/std/sync/struct.Once.html),
    /// [C++ Docs](https://en.cppreference.com/w/cpp/thread/call_once.html).
    class once final
    {
        std::atomic_flag initialized;
        std::atomic_flag busy;

        bool already_consumed() noexcept
        {
            if (this->is_completed())
                return true;

            while (this->busy.test_and_set(std::memory_order_acquire))
                thread_yield();

            if (this->initialized.test(std::memory_order_relaxed)) // LCOV_EXCL_BR_LINE: Exceptionally rare, hand verify!
            {
                this->busy.clear(std::memory_order_release); // LCOV_EXCL_LINE
                return true;                                 // LCOV_EXCL_LINE
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

        /// @brief Checks if exactly one `.call_once(...)` has successfully completed.
        [[nodiscard]] bool is_completed() const noexcept { return this->initialized.test(std::memory_order_acquire); }

        /// @brief Waits until exactly one `.call_once(...)` has successfully completed, or returns immediately if already completed.
        void wait() noexcept
        {
            while (!this->initialized.test(std::memory_order_acquire))
                thread_yield();
        }

        /// @brief Calls `func(args...)`, ensuring only one functor passed to `.call_once(...)` is ever run.
        /// @note If an exception is thrown, this `sys::once` is not consumed, and remains incomplete.
        template <typename Func>
        void call_once(Func&& func, auto&&... args) noexcept(INothrowCallable<Func&&, decltype(args)...>)
        requires requires {
            requires ICallable<Func&&, decltype(args)...>;
            requires !meta::type<std::invoke_result_t<Func&&, decltype(args)...>>::template is_from<result>();
        }
        {
            if (this->already_consumed())
                return;

            _defer([this]() noexcept -> void { this->busy.clear(std::memory_order_release); });
            _forward(func)(_forward(args)...); // LCOV_EXCL_BR_LINE: Uncovered spurious, bad instrumentation.

            this->initialized.test_and_set(std::memory_order_release);
        }

        /// @overload
        /// @return If `func(args...)` produces a `sys::result<T, Err>`, on failure, an `Err` is produced and this `sys::once` is not consumed, otherwise nothing, and this
        /// `sys::once` is marked as completed.
        /// @note If an exception is thrown, this `sys::once` is not consumed, and remains incomplete.
        /// @see `sys::once::call_once(Func&& func, Args&&... args)`
        template <typename Func, typename... Args>
        sys::result<void, typename std::invoke_result_t<Func&&, Args&&...>::err_type> call_once(Func&& func, Args&&... args) noexcept(INothrowCallable<Func&&, Args&&...>)
        requires requires {
            requires ICallable<Func&&, Args&&...>;
            requires meta::type<std::invoke_result_t<Func&&, Args&&...>>::template is_from<result>();
        }
        {
            if (this->already_consumed())
                return {};

            _defer([this]() noexcept -> void { this->busy.clear(std::memory_order_release); });
            if (auto res = _forward(func)(_forward(args)...); !res) // LCOV_EXCL_BR_LINE: Uncovered spurious, bad instrumentation.
            {
                if constexpr (!std::same_as<typename std::invoke_result_t<Func, Args&&...>::err_type, void>)
                    return res.err(); // LCOV_EXCL_BR_LINE: Uncovered spurious, bad instrumentation.
                else
                    return nullptr;
            }

            this->initialized.test_and_set(std::memory_order_release);
            return {};
        }
    };
} // namespace sys
