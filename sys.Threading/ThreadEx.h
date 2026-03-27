#pragma once

/// @file

#define NOMINMAX 1
#include <concepts>
#include <cstddef>
#include <new>
#include <tinycthread.h>
#include <type_traits>
#include <utility>
#undef NOMINMAX

#include <Destructor.h>
#include <LanguageSupport.h>
#include <Numeric.h>
#include <Result.h>
#include <ThreadingErrors.h>

namespace sys
{
    class thread;

    /// @brief Opaque, non-owning identifier for a thread, or `nullptr`.
    /// @see For more information on thread ids, see [Rust Docs](https://doc.rust-lang.org/std/thread/struct.ThreadId.html).
    struct thread_id
    {
    private:
        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running, or empty.
        thread_id(const thrd_t th, unsafe) noexcept : th(th) { }
    public:
        /* NOLINT(hicpp-explicit-conversions) */ thread_id(std::nullptr_t) noexcept { }
        thread_id(const thread_id&) noexcept = default;
        thread_id(thread_id&&) noexcept = default;
        ~thread_id() noexcept = default;

        thread_id& operator=(const thread_id&) noexcept = default;
        thread_id& operator=(thread_id&&) noexcept = default;

        [[nodiscard]] friend bool operator==(const thread_id& a, const thread_id& b) noexcept { return thrd_equal(a.th, b.th); }

        friend class sys::thread;
    };

    template <>
    class [[nodiscard]] result<thread_id, void> final : public internal::nullable_value_result<thread_id>
    {
    public:
        using internal::nullable_value_result<thread_id>::nullable_value_result;
        using internal::nullable_value_result<thread_id>::operator=;

        /* NOLINT(hicpp-explicit-conversions) */ result(std::nullptr_t) noexcept : internal::nullable_value_result<thread_id>(nullptr) { }
    };

    class managed_thread;
    thread thread_current() noexcept;

    class thread final
    {
        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running, or empty.
        thread(const thrd_t th, unsafe) noexcept : th(th) { }
    public:
        /// @brief Empty, non-joinable.
        thread() noexcept = default;
        thread(const thread&) noexcept = delete;
        thread(thread&& other) noexcept { swap(*this, other); }
        ~thread() noexcept = default;

        thread& operator=(const thread&) noexcept = delete;
        thread& operator=(thread&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Whether this thread is valid.
        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        [[nodiscard]] thread_id id() const noexcept { return { this->th, unsafe() }; }

        friend void swap(thread& a, thread& b) noexcept { std::swap(a.th, b.th); }
        friend thread sys::thread_current() noexcept;

        friend class sys::managed_thread;
    };

    /// @brief Obtains a handle to the current thread.
    inline thread thread_current() noexcept { return { thrd_current(), unsafe() }; }

    /// @brief Yields the current thread.
    inline void thread_yield() noexcept { thrd_yield(); }

    /// @brief Exits the current thread with the given return code.
    inline void thread_exit(const int ret) noexcept { thrd_exit(ret); }

    class managed_thread final
    {
        /// @brief Checks if `Func` could be the type of a global function.
        template <typename Func>
        static consteval bool is_global_func() noexcept
        {
            return std::is_function_v<std::remove_pointer_t<std::decay_t<Func>>>;
        }
        template <typename Func>
        using storage_type = std::conditional_t<managed_thread::is_global_func<Func>(), std::decay_t<Func>, std::decay_t<Func>*>;

        thrd_t th {};

        /// @warning `unsafe` because `th` must be initialized and running.
        managed_thread(const thrd_t th, unsafe) noexcept : th(th) { }
    public:
        managed_thread() noexcept = default;
        managed_thread(const managed_thread&) noexcept = delete;
        managed_thread(managed_thread&& other) noexcept { swap(*this, other); }
        ~managed_thread() noexcept
        {
            if (this->joinable())
                _contract_assert(thrd_join(this->th, nullptr) == thrd_success, "If this happens we're genuinely cooked.");
        }

        managed_thread& operator=(const managed_thread&) noexcept = delete;
        managed_thread& operator=(managed_thread&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Trampoline off a `sys::managed_thread` executing `func`.
        template <typename Func>
        static result<managed_thread, threading_error> ctor(Func&& func) noexcept(noexcept(auto(std::forward<Func>(func)())))
        requires requires {
            { func() };
        }
        {
            thrd_t th {};

            storage_type<Func> f = [&]() noexcept -> storage_type<Func>
            {
                if constexpr (!managed_thread::is_global_func<Func>())
                    return new(std::nothrow) std::decay_t<Func>(std::forward<Func>(func)); // NOLINT(cppcoreguidelines-owning-memory)
                else
                    return func;
            }();
            if constexpr (managed_thread::is_global_func<Func>())
                _retif(threading_error::invalid_argument, !f);
            else
                _retif(threading_error::oom, !f);

            sys::optional_destructor releaseFunc = [&]() noexcept -> void
            {
                if constexpr (!managed_thread::is_global_func<Func>())
                    delete f; // NOLINT(cppcoreguidelines-owning-memory)
            };

            if (thrd_create(&th, [](void* arg) noexcept(noexcept(func())) -> int
            {
                int ret = 0;
                std::decay_t<Func> func = [&]() -> std::decay_t<Func>
                {
                    if constexpr (!managed_thread::is_global_func<Func>())
                        return *(_as(std::decay_t<Func>*, arg));
                    else
                        return _asr(std::decay_t<Func>, arg);
                }();

                try
                {
                    if constexpr (std::convertible_to<std::invoke_result_t<std::decay_t<Func>>, int>)
                        ret = _as(int, func());
                    else
                        (void)func();
                }
                catch (...)
                {
                    return sys::bsentinel<int>();
                }

                return ret;
            }, _asr(void*, f)) != thrd_success)
                return threading_error::init_failed;

            releaseFunc.release(unsafe());
            return managed_thread(th, unsafe());
        }

        /// @brief Whether this managed thread is valid.
        [[nodiscard]] explicit operator bool() const noexcept { return this->th != thrd_t {}; }
        [[nodiscard]] bool joinable() const noexcept { return this->th != thrd_t {} && !thrd_equal(this->th, thrd_current()); }

        [[nodiscard]] thread thread() const noexcept { return { this->th, unsafe() }; }

        result<sys::integer<int>, threading_error> join() noexcept
        {
            _retif(threading_error::invalid_operation, !this->joinable());

            sys::integer<int> res = 0;
            if (thrd_join(this->th, &*res) != thrd_success)
                return threading_error::operation_failed;
            this->th = thrd_t {};
            return res;
        }
        void detach() noexcept
        {
            _retif(, !this->joinable());

            (void)thrd_detach(this->th);
            this->th = thrd_t {};
        }

        friend void swap(managed_thread& a, managed_thread& b) noexcept { std::swap(a.th, b.th); }
    };
} // namespace sys

#undef timespec // NOLINT(misc-include-cleaner)
