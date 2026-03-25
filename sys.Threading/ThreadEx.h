#pragma once

/// @file

#include <concepts>
#include <new>
#include <tinycthread.h>
#include <type_traits>
#include <utility>

#include <Destructor.h>
#include <LanguageSupport.h>
#include <Numeric.h>
#include <Result.h>
#include <ThreadingErrors.h>

namespace sys
{
    class thread;

    /// @brief Opaque, non-owning identifier for a thread, or `nullptr`.
    /// @see For more information on thread IDs, see [Rust Docs](https://doc.rust-lang.org/std/thread/struct.ThreadId.html).
    struct thread_id
    {
    private:
        thrd_t th {};

        thread_id(const thrd_t th, unsafe) noexcept : th(th) { }
    public:
        /* NOLINT(hicpp-explicit-conversions) */ thread_id(std::nullptr_t) noexcept { }
        thread_id(const thread_id&) noexcept = default;
        thread_id(thread_id&&) noexcept = default;
        ~thread_id() = default;

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
    };

    thread thread_current() noexcept;

    class thread final
    {
        /// @brief Checks if `Func` could be the type of a global function.
        template <typename Func>
        static consteval bool is_global_func() noexcept
        {
            return std::is_function_v<std::remove_pointer_t<std::decay_t<Func>>>;
        }
        template <typename Func>
        using storage_type = std::conditional_t<thread::is_global_func<Func>(), std::decay_t<Func>, std::decay_t<Func>*>;

        thrd_t th {};
        bool is_ref = true;

        /// @warning `unsafe` because `th` must be initialized and running.
        thread(const thrd_t th, const bool is_ref, unsafe) noexcept : th(th), is_ref(is_ref) { }
    public:
        friend void swap(thread& a, thread& b) noexcept(noexcept(std::swap(a.th, b.th)))
        {
            std::swap(a.th, b.th);
            std::swap(a.is_ref, b.is_ref);
        }
        friend thread sys::thread_current() noexcept;

        /// @brief Empty, non-joinable.
        thread() noexcept = default;
        thread(const thread&) = delete;
        thread(thread&& other) noexcept(noexcept(swap(*this, other))) { swap(*this, other); }
        ~thread() noexcept
        {
            if (this->joinable())
                _contract_assert(thrd_join(this->th, nullptr) == thrd_success, "If this happens we're genuinely cooked.");
        }

        thread& operator=(const thread&) = delete;
        thread& operator=(thread&& other) noexcept(noexcept(swap(*this, other)))
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Create a `sys::thread` with `func`.
        template <typename Func>
        static result<thread, threading_error> ctor(Func&& func) noexcept(noexcept(auto(std::forward<Func>(func)())))
        requires requires {
            { func() };
        }
        {
            thrd_t th {};

            storage_type<Func> f = [&] noexcept
            {
                if constexpr (!thread::is_global_func<Func>())
                    return new(std::nothrow) std::decay_t<Func>(std::forward<Func>(func)); // NOLINT(cppcoreguidelines-owning-memory)
                else
                    return func;
            }();
            if constexpr (thread::is_global_func<Func>())
                _retif(threading_error::invalid_argument, !f);
            else
                _retif(threading_error::oom, !f);

            sys::optional_destructor releaseFunc = [&] noexcept
            {
                if constexpr (!thread::is_global_func<Func>())
                    delete f; // NOLINT(cppcoreguidelines-owning-memory)
            };

            if (thrd_create(&th, [](void* arg) noexcept(noexcept(func())) -> int
            {
                int ret = 0;
                std::decay_t<Func> func = [&]()
                {
                    if constexpr (!thread::is_global_func<Func>())
                        return *(_as(std::decay_t<Func>*, arg));
                    else
                        return _asr(std::decay_t<Func>, arg);
                }();

                try
                {
                    if constexpr (requires {
                                      { func() } -> std::convertible_to<int>;
                                  })
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
            return thread(th, false, unsafe());
        }

        /// @brief Whether this thread is joinable.
        [[nodiscard]] explicit operator bool() const noexcept { return this->joinable(); }
        [[nodiscard]] bool joinable() const noexcept { return !this->is_ref && this->th != thrd_t {} && !thrd_equal(this->th, thrd_current()); }

        [[nodiscard]] thread_id id() const noexcept { return { this->th, unsafe() }; }

        result<sys::integer<int>, threading_error> join() noexcept
        {
            _retif(threading_error::invalid_operation, !this->joinable());

            sys::integer<int> res = 0;
            if (thrd_join(this->th, &*res) != thrd_success)
                return threading_error::operation_failed;
            this->th = thrd_t {};
            this->is_ref = true;
            return res;
        }
        void detach() noexcept
        {
            _retif(, !this->joinable());

            (void)thrd_detach(this->th);
            this->th = thrd_t {};
            this->is_ref = true;
        }
    };

    /// @brief Obtains a handle to the current thread.
    inline thread thread_current() noexcept { return { thrd_current(), true, unsafe() }; }

    /// @brief Yields the current thread.
    inline void thread_yield() noexcept { thrd_yield(); }

    /// @brief Exits the current thread with the given return code.
    inline void thread_exit(const int ret) noexcept { thrd_exit(ret); }
} // namespace sys

#undef timespec // NOLINT(misc-include-cleaner)
