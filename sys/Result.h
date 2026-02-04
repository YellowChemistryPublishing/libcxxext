#pragma once

#include <algorithm>
#include <concepts>
#include <coroutine>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <Exception.h>
#include <LanguageSupport.h>
#include <inline/Integer.inl>

// NOLINTBEGIN(bugprone-macro-parentheses)
#define _res_movret(out, res_xval)          \
    do                                      \
    {                                       \
        auto _result = std::move(res_xval); \
        if (!_result)                       \
            return _result.err();           \
        out = _result.move();               \
    }                                       \
    while (false)
#define _res_movcoret(out, res_xval)        \
    do                                      \
    {                                       \
        auto _result = std::move(res_xval); \
        if (!_result)                       \
            co_return _result.err();        \
        out = _result.move();               \
    }                                       \
    while (false)
// NOLINTEND(bugprone-macro-parentheses)

namespace sys
{
    enum class result_status : byte
    {
        ok,
        error,
        empty
    };
} // namespace sys

namespace sys::internal
{
    template <typename T, typename Err>
    struct result_awaiter;

    template <typename T>
    using result_storage_type = std::conditional_t<!std::is_reference_v<T>, std::remove_const_t<T>, std::remove_reference_t<T>*>;

    template <template <typename T, typename Err> class Result, typename T, typename Err>
    struct result_b
    {
    private:
        using result_type = Result<T, Err>;
    public:
        /// @brief Whether the result is good.
        constexpr explicit operator bool() const noexcept { return _as(const result_type*, this)->status == result_status::ok; }

        /// @brief Returns an awaiter to enable short-circuiting, akin to rustlang's `operator?`.
        _inline_always result_awaiter<T, Err> operator co_await() { return result_awaiter<T, Err>(*_as(result_type*, this)); }

        /// @brief Takes the value if the result is good.
        /// @return The value held by the result.
        constexpr T move()
        {
            _contract_assert(_as(result_type*, this)->status == result_status::ok && "Taking value for a bad result!");
            if constexpr (std::is_reference_v<T>)
                return *_as(result_type*, this)->value;
            else
                return _as(result_type*, this)->value;
        }
        constexpr T expect()
        {
            if (_as(result_type*, this)->status == result_status::ok)
            {
                if constexpr (std::is_reference_v<T>)
                    return *_as(result_type*, this)->value;
                else
                    return _as(result_type*, this)->value;
            }
            else
                _throw(contract_violation_exception("Result is not ok!"));
        }
    };
} // namespace sys::internal

namespace sys
{
    /// @brief A result type that can hold either a value or an error.
    /// @note Pass `byref`.
    template <typename T, typename Err = void>
    requires (!std::same_as<T, Err> && std::same_as<T, std::remove_cvref_t<T>> && std::same_as<Err, std::remove_cvref_t<Err>>)
    struct result : internal::result_b<result, T, Err>
    {
        /// @brief Constructs a result with a value.
        constexpr result(T&& value) : status(result_status::ok) // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
        {
            new(&this->value) internal::result_storage_type<T>(std::move(value));
        }
        /// @brief Constructs a result with an error.
        constexpr result(Err&& error) : status(result_status::error) // NOLINT(hicpp-explicit-conversions)
        {
            new(&this->error) internal::result_storage_type<Err>(std::move(error));
        }
        constexpr result(const result&) = delete;
        constexpr result(result&& other) noexcept : status(result_status::empty) { swap(*this, other); }
        constexpr ~result()
        {
            switch (this->status)
            {
            case result_status::ok:
                using result_storage_type_t = internal::result_storage_type<T>;
                if constexpr (!std::is_reference_v<T>)
                    this->value.~result_storage_type_t();
                break;
            case result_status::error:
                using result_storage_type_e = internal::result_storage_type<Err>;
                if constexpr (!std::is_reference_v<Err>)
                    this->error.~result_storage_type_e();
                break;
            case result_status::empty:
            default:; // Do nothing.
            }
        }

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        [[nodiscard]] constexpr Err err() const
        {
            _contract_assert(this->status == result_status::error && "Taking error for a good or empty result!");
            return this->error;
        }

        constexpr void swap(result& a, result& b)
        {
            using std::swap;

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (&a != &b && (a.status == result_status::ok || a.status == result_status::error || b.status == result_status::ok || b.status == result_status::error))
                std::swap_ranges(_asr(byte*, &a.value), _asr(byte*, &a.value) + std::max(sizeof(internal::result_storage_type<T>), sizeof(internal::result_storage_type<Err>)),
                                 _asr(byte*, &b.value));
            swap(a.status, b.status);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        friend struct sys::internal::result_b<result, T, Err>;
    private:
        union
        {
            internal::result_storage_type<T> value;
            internal::result_storage_type<Err> error;
        };
        result_status status;
    };

    template <typename T>
    struct result<T, void> : internal::result_b<result, T, void>
    {
        /// @brief Constructs a result with a value.
        /// @param value Value to move into the result.
        constexpr result(T&& value) : status(result_status::ok) // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
        {
            new(&this->value) internal::result_storage_type<T>(std::move(value));
        }
        constexpr result(std::nullptr_t) : status(result_status::error) // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
        { }
        constexpr result(const result&) = delete;
        constexpr result(result&& other) noexcept : status(result_status::empty) // NOLINT(hicpp-member-init)
        {
            swap(*this, other);
        }
        ~result()
        {
            if (this->status == result_status::ok)
                this->value.~T();
        }

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        friend constexpr void swap(result& a, result& b) noexcept
        {
            using std::swap;

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (&a != &b && (a.status == result_status::ok || b.status == result_status::ok))
                std::swap_ranges(_asr(byte*, &a.value), _asr(byte*, &a.value) + sizeof(internal::result_storage_type<T>), _asr(byte*, &b.value));
            swap(a.status, b.status);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        friend struct sys::internal::result_b<result, T, void>;
    private:
        union
        {
            byte _;
            internal::result_storage_type<T> value;
        };
        result_status status;
    };

    /// @brief Awaiter to enable short-circuiting, akin to rustlang's `operator?`.
    template <typename T, typename Err>
    struct result_awaiter
    {
        [[nodiscard]] _inline_always constexpr bool await_ready() const noexcept { return res; }
        template <typename Promise>
        _inline_always constexpr void await_suspend(std::coroutine_handle<Promise> parent)
        {
            if constexpr (!std::is_same_v<Err, void>)
                parent.promise().return_value(res.err());
            else
                []<bool _false = false> { static_assert(_false, "`result<...>::operator?` requires `typename Err` to be returnable in the current scope!"); }();

            if constexpr (requires { parent.promise().continuation.resume(); })
                parent.promise().continuation.resume();
        }
        _inline_always constexpr T await_resume() const noexcept(std::is_same_v<T, void>) { return res.move(); }

        friend struct sys::result<T, Err>;
    private:
        result<T, Err>& res; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    };
} // namespace sys
