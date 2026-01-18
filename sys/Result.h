#pragma once

#include <algorithm>
#include <coroutine> // NOLINT(misc-include-cleaner)
#include <cstddef>
#include <type_traits>

#include <Exception.h>
#include <LanguageSupport.h>

// NOLINTBEGIN(bugprone-macro-parentheses)
#define _fence_result_return(rValRef, out) \
    do                                     \
    {                                      \
        auto __result = rValRef;           \
        if (!__result)                     \
            return __result.err();         \
        out = __result.move();             \
    }                                      \
    while (false)
#define _fence_result_co_return(rValRef, out) \
    do                                        \
    {                                         \
        auto __result = rValRef;              \
        if (!__result)                        \
            co_return __result.err();         \
        out = __result.move();                \
    }                                         \
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

    template <typename T, typename Err>
    struct result_awaiter;

    template <typename T>
    using result_storage_type = std::conditional_t<!std::is_reference_v<T>, std::remove_const_t<T>, std::remove_reference_t<T>*>;

    template <template <typename T, typename Err> class Result, typename T, typename Err>
    struct result_b
    {
        /// @brief Whether the result is good.
        constexpr explicit operator bool() const noexcept
        {
            using this_type = type_with_qual_ref_from<result_type, std::remove_pointer_t<decltype(this)>>*;
            return _as(this_type, this)->status == result_status::ok;
        }

        _inline_always result_awaiter<T, Err> operator co_await()
        {
            return result_awaiter<T, Err>(*this);
        }

        /// @brief Takes the value if the result is good.
        /// @return The value held by the result.
        constexpr T move()
        {
            using this_type = type_with_qual_ref_from<result_type, std::remove_pointer_t<decltype(this)>>*;

            _fence_contract_enforce(_as(this_type, this)->status == result_status::ok && "Taking value for a bad result!");
            if constexpr (std::is_reference_v<T>)
                return *_as(this_type, this)->value;
            else
                return _as(this_type, this)->value;
        }
        constexpr T expect()
        {
            using this_type = type_with_qual_ref_from<result_type, std::remove_pointer_t<decltype(this)>>*;

            if (_as(this_type, this)->status == result_status::ok)
            {
                if constexpr (std::is_reference_v<T>)
                    return *_as(this_type, this)->value;
                else
                    return _as(this_type, this)->value;
            }
            else
                _throw(contract_violation_exception("Result is not ok!"));
        }
    private:
        using result_type = Result<T, Err>;
    };

    /// @brief A result type that can hold either a value or an error.
    /// @tparam T The type of the value to hold.
    /// @tparam Err The type of the error to hold.
    /// @attention `T`, `Err` must be trivially destructible after moved from.
    template <typename T, typename Err = void>
    struct result : result_b<result, T, Err>
    {
        /// @brief Constructs a result with a value.
        /// @param value Value to move into the result.
        constexpr result(T&& value) // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
        {
            if constexpr (std::is_reference_v<T>)
                this->value = &value;
            else
                new(&this->value) result_storage_type<T>(std::move(value));
            this->status = result_status::ok;
        }
        /// @brief Constructs a result with an error.
        /// @param error Error to move into the result.
        constexpr result(Err&& error) : status(result_status::error) // NOLINT(hicpp-explicit-conversions)
        {
            if constexpr (std::is_reference_v<T>)
                this->error = &error;
            else
                new(&this->error) result_storage_type<Err>(std::move(error));
        }
        constexpr result(const result&) = delete;
        constexpr result(result&& other) noexcept : status(result_status::empty)
        {
            swap(*this, other);
        }
        constexpr ~result()
        {
            switch (this->status)
            {
            case result_status::ok:
                if constexpr (!std::is_reference_v<T>)
                    this->value.~result_storage_type<T>();
                break;
            case result_status::error:
                if constexpr (!std::is_reference_v<Err>)
                    this->error.~result_storage_type<Err>();
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

        constexpr Err err() const
        {
            _fence_contract_enforce(this->status == result_status::error && "Taking error for a good or empty result!");
            return this->error;
        }

        constexpr void swap(result& a, result& b)
        {
            using std::swap;

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (&a != &b && (a.status == result_status::ok || a.status == result_status::error || b.status == result_status::ok || b.status == result_status::error))
                std::swap_ranges(_asr(byte*, &a.value), _asr(byte*, &a.value) + std::max(sizeof(result_storage_type<T>), sizeof(result_storage_type<Err>)), _asr(byte*, &b.value));
            swap(a.status, b.status);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        friend struct sys::result_b<result, T, Err>;
    private:
        union
        {
            result_storage_type<T> value;
            result_storage_type<Err> error;
        };
        result_status status;
    };

    template <typename T>
    struct result<T, void> : result_b<result, T, void>
    {
        /// @brief Constructs a result with a value.
        /// @param value Value to move into the result.
        constexpr result(T&& value) // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
        {
            if constexpr (std::is_reference_v<T>)
                this->value = &value;
            else
                new(&this->value) result_storage_type<T>(std::move(value));
            this->status = result_status::ok;
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
            if constexpr (!std::is_reference_v<T>)
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
                std::swap_ranges(_asr(byte*, &a.value), _asr(byte*, &a.value) + sizeof(result_storage_type<T>), _asr(byte*, &b.value));
            swap(a.status, b.status);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        friend struct sys::result_b<result, T, void>;
    private:
        union
        {
            byte _;
            result_storage_type<T> value;
        };
        result_status status;
    };

    /// @brief Awaiter to enable short-circuiting, akin to rustlang's `operator?`.
    /// @tparam T The type of the value to hold.
    /// @tparam Err The type of the error to hold.
    template <typename T, typename Err>
    struct result_awaiter
    {
        [[nodiscard]] _inline_always constexpr bool await_ready() const noexcept
        {
            return res;
        }
        template <typename Promise>
        _inline_always constexpr void await_suspend(std::coroutine_handle<Promise> parent)
        {
            if constexpr (!std::is_same_v<Err, void>)
                parent.promise().return_value(res.takeError());
            else
                []<bool _false = false>
                {
                    static_assert(_false, "`result<...>::operator?` requires `typename Err` to be returnable in the current scope!");
                }();

            if constexpr (requires { parent.promise().continuation.resume(); })
                parent.promise().continuation.resume();
        }
        _inline_always constexpr T await_resume() const noexcept(std::is_same_v<T, void>)
        {
            return res.takeValue();
        }

        friend struct sys::result<T, Err>;
    private:
        result<T, Err>& res; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    };
} // namespace sys
