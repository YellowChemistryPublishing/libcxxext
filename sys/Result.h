#pragma once

#include <algorithm>
#include <coroutine>
#include <cstddef>
#include <print>
#include <type_traits>

#include <CompilerWarnings.h>
#include <Exception.h>
#include <LanguageSupport.h>

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
        constexpr operator bool() const noexcept
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
            _as(this_type, this)->status = result_status::empty;
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
                _as(this_type, this)->status = result_status::empty;
                if constexpr (std::is_reference_v<T>)
                    return *_as(this_type, this)->value;
                else
                    return _as(this_type, this)->value;
            }
            else
                _throw(contract_violation_exception("Result is not ok!"));
        }
    private:
        using result_type = result<T, Err>;
    };

    /// @brief A result type that can hold either a value or an error.
    /// @tparam T The type of the value to hold.
    /// @tparam Err The type of the error to hold.
    template <typename T, typename Err = void>
    struct result : result_b<result, T, Err>
    {
        /// @brief Constructs a result with a value.
        /// @param value Value to move into the result.
        constexpr result(T&& value)
        {
            if constexpr (std::is_reference_v<T>)
                this->value = &value;
            else
                new(&this->value) result_storage_type<T>(std::forward<T>(value));
            this->status = result_status::ok;
        }
        /// @brief Constructs a result with an error.
        /// @param error Error to move into the result.
        constexpr result(Err&& error) : status(result_status::error)
        {
            if constexpr (std::is_reference_v<T>)
                this->error = &error;
            else
                new(&this->error) result_storage_type<Err>(std::forward<Err>(error));
        }
        constexpr result(result&& other)
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
            }
        }

        constexpr Err err()
        {
            _fence_contract_enforce(this->status == result_status::error && "Taking error for a good or empty result!");
            this->status = result_status::empty;
            return this->template obtainStorage<Err>();
        }

        constexpr void swap(result& a, result& b)
        {
            using std::swap;

            if (&a != &b && (a.status == result_status::ok || a.status == result_status::error || b.status == result_status::ok || b.status == result_status::error))
                std::swap_ranges(_asr(byte*, &a.value), _asr(byte*, &a.value) + std::max(sizeof(result_storage_type<T>), sizeof(result_storage_type<Err>)), _asr(byte*, &b.value));
            swap(a.status, b.status);
        }

        friend struct sys::result_b<result, T, Err>;
    private:
        union
        {
            result_storage_type<T> value;
            result_storage_type<Err> error;
        };
        result_status status = result_status::empty;
    };

    template <typename T>
    struct result<T, void> : result_b<result, T, void>
    {
        /// @brief Constructs a result with a value.
        /// @param value Value to move into the result.
        constexpr result(T&& value)
        {
            if constexpr (std::is_reference_v<T>)
                this->value = &value;
            else
                new(&this->value) result_storage_type<T>(std::forward<T>(value));
            this->status = result_status::ok;
        }
        constexpr result(std::nullptr_t) : status(result_status::error)
        { }
        constexpr result(result&& other)
        {
            swap(*this, other);
        }
        constexpr ~result()
        {
            if constexpr (!std::is_reference_v<T>)
                if (this->status == result_status::ok)
                    _asr(result_storage_type<T>*, this->storage)->~T();
        }

        constexpr void swap(result& a, result& b)
        {
            using std::swap;

            if (&a != &b && (a.status == result_status::ok || b.status == result_status::ok))
                std::swap_ranges(_asr(byte*, &a.value), _asr(byte*, &a.value) + sizeof(result_storage_type<T>), _asr(byte*, &b.value));
            swap(a.status, b.status);
        }

        friend struct sys::result_b<result, T, void>;
    private:
        union
        {
            byte none;
            result_storage_type<T> value;
        };
        result_status status = result_status::empty;
    };

    /// @brief Awaiter to enable short-circuiting, à la rustlang `operator?`.
    /// @tparam T The type of the value to hold.
    /// @tparam Err The type of the error to hold.
    template <typename T, typename Err>
    struct result_awaiter
    {
        _inline_always constexpr bool await_ready() const noexcept
        {
            return res;
        }
        template <typename Promise>
        _inline_always constexpr void await_suspend(std::coroutine_handle<Promise> parent)
        {
            if constexpr (!std::is_same<Err, void>::value)
                parent.promise().return_value(res.takeError());
            else
                []<bool _false = false>
                {
                    static_assert(_false, "`result<...>::operator?` requires `typename Err` to be returnable in the current scope!");
                }();

            if constexpr (requires { parent.promise().continuation.resume(); })
                parent.promise().continuation.resume();
        }
        _inline_always constexpr T await_resume() const noexcept(std::is_same<T, void>::value)
        {
            return res.takeValue();
        }

        friend struct sys::result<T, Err>;
    private:
        result<T, Err>& res;
    };
} // namespace sys
