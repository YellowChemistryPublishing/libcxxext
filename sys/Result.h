#pragma once

/// @file Result.h

#include <algorithm>
#include <cassert>
#include <concepts>
#include <coroutine>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <Exception.h>
#include <LanguageSupport.h>
#include <RecurringTemplate.h>
#include <inline/Integer.inl>

// NOLINTBEGIN(bugprone-macro-parentheses)
/// @def _res_movret(out_decl, res_xval)
/// @brief Initialize `out_decl` with `res_xval`, early-returning if `res_xval` is an error.
#define _res_movret(out_decl, res_xval)                        \
    auto _ppcat(_result, __LINE__) = res_xval;                 \
    if (!_ppcat(_result, __LINE__))                            \
    {                                                          \
        return [](auto&& res)                                  \
        {                                                      \
            if constexpr (requires { res.err(); })             \
                return std::forward<decltype(res)>(res).err(); \
            else                                               \
                return nullptr;                                \
        }(std::move(_ppcat(_result, __LINE__)));               \
    }                                                          \
    out_decl = _ppcat(_result, __LINE__).move();
/// @def _res_movcoret(out_decl, res_xval)
/// @brief Initialize `out_decl` with `res_xval`, early-coroutine-returning if `res_xval` is an error.
#define _res_movcoret(out_decl, res_xval)                      \
    auto _ppcat(_result, __LINE__) = res_xval;                 \
    if (!_ppcat(_result, __LINE__))                            \
    {                                                          \
        co_return [](auto&& res)                               \
        {                                                      \
            if constexpr (requires { res.err(); })             \
                return std::forward<decltype(res)>(res).err(); \
            else                                               \
                return nullptr;                                \
        }(std::move(_ppcat(_result, __LINE__)));               \
    }                                                          \
    out_decl = _ppcat(_result, __LINE__).move();
// NOLINTEND(bugprone-macro-parentheses)

namespace sys::internal
{
    /// @internal
    /// @brief Status of a result.
    enum class result_status : byte
    {
        ok,
        error,
        empty
    };

    template <typename T, typename Err>
    struct result_awaiter;

    /// @internal
    /// @brief Type to use in result storage to represent `T`.
    template <typename T>
    using result_storage_type = std::conditional_t<!std::is_reference_v<T>, std::remove_cv_t<T>, std::remove_reference_t<T>*>;

    /// @internal
    /// @brief Shared functionality for result types.
    template <template <typename T, typename Err> class Result, typename T, typename Err>
    struct result_b : recurring_template<Result<T, Err>>
    {
    protected:
        result_b() noexcept = default;
    public:
        /// @brief Whether the result is good.
        constexpr explicit operator bool() const noexcept { return this->downcast()->status == result_status::ok; }

        /// @brief Returns an awaiter to enable short-circuiting, akin to rustlang's `operator?`.
        _inline_always result_awaiter<T, Err> operator co_await() { return result_awaiter<T, Err>(*this->downcast()); }
    };
    /// @internal
    /// @brief Shared functionality for result types that are good.
    template <template <typename T, typename Err> class Result, typename T, typename Err>
    struct result_b_ok : recurring_template<Result<T, Err>>
    {
    protected:
        result_b_ok() noexcept = default;
    public:
        /// @brief Takes the value if the result is good.
        /// @pre `*this == true`
        constexpr T move() noexcept(noexcept(T(std::declval<T&&>())))
        {
            // We use regular assert here as we expect this to be called in a noexcept context.
            // Only a flagrant violation of result semantics would cause this to fail.
            assert(this->downcast()->status == result_status::ok && "Taking value for a bad result!");
            if constexpr (std::is_reference_v<T>)
                return *this->downcast()->value;
            else
                return std::move(this->downcast()->value);
        }

        /// @brief `this->move()` carrying contract of being a good result.
        /// @pre `*this == true`
        constexpr T expect()
        {
            if (this->downcast()->status == result_status::ok)
            {
                if constexpr (std::is_reference_v<T>)
                    return *this->downcast()->value;
                else
                    return std::move(this->downcast()->value);
            }
            else
                _throw(contract_violation_exception("Result is not ok!"));
        }
        /// @brief `this->move()` if the result is good, otherwise `other`.
        constexpr T move_or(T&& other) noexcept(noexcept(T(std::declval<T&&>())))
        {
            if (this->downcast()->status != result_status::ok)
                return std::move(other);
            return std::move(this->downcast()->value);
        }
    };
    /// @internal
    /// @brief Shared functionality for result types that are bad.
    template <template <typename T, typename Err> class Result, typename T, typename Err>
    struct result_b_err : recurring_template<Result<T, Err>>
    {
    protected:
        result_b_err() noexcept = default;
    public:
        /// @brief Take the error of a bad result.
        /// @pre `*this == false`
        [[nodiscard]] constexpr Err err()
        {
            _contract_assert(this->downcast()->status == result_status::error && "Taking error for a good or empty result!");
            return std::move(this->downcast()->error);
        }
    };
} // namespace sys::internal

namespace sys
{
    /// @brief Tag type to force construction of a bad result.
    struct error_tag final
    { };

    /// @brief Concept for types that can be stored in a result.
    template <typename T>
    concept IResultStorable = !std::same_as<std::remove_cvref_t<T>, error_tag> && (std::is_reference_v<T> || (!std::is_reference_v<T> && std::same_as<T, std::remove_cvref_t<T>>));

    /// @brief A monadic type that can hold either a value or an error.
    /// @details Like the one in rustlang!
    /// @note Pass `byref`.
    template <IResultStorable T, IResultStorable Err = void>
    struct [[nodiscard]] result final : internal::result_b<result, T, Err>, internal::result_b_ok<result, T, Err>, internal::result_b_err<result, T, Err>
    {
    private:
        union
        {
            internal::result_storage_type<T> value;
            internal::result_storage_type<Err> error;
        };
        internal::result_status status = internal::result_status::empty;
    public:
        // NOLINTBEGIN(hicpp-explicit-conversions)

        /// @brief Constructs a result with a value.
        constexpr result(const T& val) noexcept(noexcept(T(std::declval<const T&>())))
        requires requires { internal::result_storage_type<T>(val); }
            : status(internal::result_status::ok)
        {
            new(&this->value) internal::result_storage_type<T>(val);
        }
        /// @brief Constructs a result with a value.
        constexpr result(T&& val) noexcept(noexcept(T(std::declval<T&&>())))
        requires requires { internal::result_storage_type<T>(std::move(val)); }
            : status(internal::result_status::ok)
        {
            new(&this->value) internal::result_storage_type<T>(std::move(val));
        }
        /// @brief Inplace constructs a result with a value.
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(T(std::declval<Args&&>()...)))
        requires requires { internal::result_storage_type<T>(std::forward<Args>(args)...); }
            : status(internal::result_status::ok)
        {
            new(&this->value) internal::result_storage_type<T>(std::forward<Args>(args)...);
        }
        /// @brief Inplace constructs a result with an error.
        template <typename... Args>
        constexpr result(error_tag, Args&&... args) noexcept(noexcept(Err(std::declval<Args&&>()...)))
        requires (!requires { internal::result_storage_type<T>(std::forward<Args>(args)...); } && requires { internal::result_storage_type<Err>(std::forward<Args>(args)...); })
            : status(internal::result_status::error)
        {
            new(&this->error) internal::result_storage_type<Err>(std::forward<Args>(args)...);
        }
        /// @brief Constructs a result with an error.
        constexpr result(const Err& err) noexcept(noexcept(Err(std::declval<const Err&>())))
        requires requires { internal::result_storage_type<Err>(err); }
            : status(internal::result_status::error)
        {
            new(&this->error) internal::result_storage_type<Err>(err);
        }
        /// @brief Inplace constructs a result with an error.
        constexpr result(Err&& err) noexcept(noexcept(Err(std::declval<Err&&>())))
        requires requires { internal::result_storage_type<Err>(std::move(err)); }
            : status(internal::result_status::error)
        {
            new(&this->error) internal::result_storage_type<Err>(std::move(err));
        }
        /// @brief Inplace constructs a result with an error.
        /// @see `sys::result<T, Err>::result(Args&&...)`
        /// @note Participates in overload resolution only if the arguments cannot construct a `T`.
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(Err(std::declval<Args&&>()...)))
        requires (!requires { internal::result_storage_type<T>(std::forward<Args>(args)...); } && requires { internal::result_storage_type<Err>(std::forward<Args>(args)...); })
            : result(error_tag(), std::forward<Args>(args)...)
        { }

        // NOLINTEND(hicpp-explicit-conversions)

        constexpr result(const result&) = delete;
        /// @brief Moveable.
        constexpr result(result&& other) noexcept { swap(*this, other); }
        constexpr ~result()
        {
            switch (this->status)
            {
            case internal::result_status::ok:
                using result_storage_type_t = internal::result_storage_type<T>;
                if constexpr (!std::is_reference_v<T>)
                    this->value.~result_storage_type_t();
                break;
            case internal::result_status::error:
                using result_storage_type_e = internal::result_storage_type<Err>;
                if constexpr (!std::is_reference_v<Err>)
                    this->error.~result_storage_type_e();
                break;
            case internal::result_status::empty:
            default:; // Do nothing.
            }
        }

        result& operator=(const result&) = delete;
        /// @brief Move-assignable.
        result& operator=(result&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Swappable.
        constexpr void swap(result& a, result& b)
        {
            using std::swap;

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (&a != &b &&
                (a.status == internal::result_status::ok || a.status == internal::result_status::error || b.status == internal::result_status::ok ||
                 b.status == internal::result_status::error))
                std::swap_ranges(_asr(byte*, &a.value), _asr(byte*, &a.value) + std::max(sizeof(internal::result_storage_type<T>), sizeof(internal::result_storage_type<Err>)),
                                 _asr(byte*, &b.value));
            swap(a.status, b.status);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        friend struct sys::internal::result_b<result, T, Err>;
        friend struct sys::internal::result_b_ok<result, T, Err>;
        friend struct sys::internal::result_b_err<result, T, Err>;
    };

    /// @brief Specialization of `sys::result<...>` with a unit error type.
    /// @details For a result with a single possible error state, iow. a valueless error.
    template <IResultStorable T>
    requires (!std::same_as<std::remove_cvref_t<T>, std::nullptr_t>)
    struct [[nodiscard]] result<T, void> final : internal::result_b<result, T, void>, internal::result_b_ok<result, T, void>
    {
    private:
        union
        {
            byte _;
            internal::result_storage_type<T> value;
        };
        internal::result_status status = internal::result_status::empty;
    public:
        /// @brief Inplace constructs an ok result.
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(T(std::declval<Args&&>()...))) // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
        requires requires { internal::result_storage_type<T>(std::forward<Args>(args)...); }
            : status(internal::result_status::ok)
        {
            new(&this->value) internal::result_storage_type<T>(std::forward<Args>(args)...);
        }
        /// @brief Contruct an error result.
        constexpr result(std::nullptr_t) noexcept : status(internal::result_status::error) // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
        { }
        constexpr result(const result&) = delete;
        /// @brief Moveable.
        constexpr result(result&& other) noexcept // NOLINT(hicpp-member-init)
        {
            swap(*this, other);
        }
        ~result()
        {
            if (this->status == internal::result_status::ok)
                this->value.~T();
        }

        result& operator=(const result&) = delete;
        /// @brief Move-assignable.
        result& operator=(result&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Swappable.
        friend constexpr void swap(result& a, result& b) noexcept
        {
            using std::swap;

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (&a != &b && (a.status == internal::result_status::ok || b.status == internal::result_status::ok))
                std::swap_ranges(_asr(byte*, &a.value), _asr(byte*, &a.value) + sizeof(internal::result_storage_type<T>), _asr(byte*, &b.value));
            swap(a.status, b.status);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        friend struct sys::internal::result_b<result, T, void>;
        friend struct sys::internal::result_b_ok<result, T, void>;
    };

    /// @brief Specialization of `sys::result<...>` that holds no value if ok.
    /// @details
    template <typename Err>
    struct [[nodiscard]] result<void, Err> final : internal::result_b<result, void, Err>, internal::result_b_err<result, void, Err>
    {
    private:
        union
        {
            byte _;
            internal::result_storage_type<Err> error;
        };
        internal::result_status status;
    public:
        constexpr result() noexcept : status(internal::result_status::ok) // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
        { }
        /// @brief Inplace constructs a result with an error.
        template <typename... Args>
        constexpr result(error_tag, Args&&... args) noexcept(noexcept(Err(std::declval<Args&&>()...))) : // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
            status(internal::result_status::error)
        {
            new(&this->error) internal::result_storage_type<Err>(std::forward<Args>(args)...);
        }
        /// @brief Inplace constructs a result with an error.
        /// @see `sys::result<T, Err>::result(Args&&...)`
        /// @note Participates in overload resolution only if the arguments cannot construct a `T`.
        template <typename... Args>
        requires (sizeof...(Args) > 0uz)
        constexpr result(Args&&... args) noexcept(noexcept(Err(std::declval<Args&&>()...))) : // NOLINT(hicpp-explicit-conversions, hicpp-member-init)
            result(error_tag(), std::forward<Args>(args)...)
        { }
        constexpr result(const result&) = delete;
        /// @brief Moveable.
        constexpr result(result&& other) noexcept : status(internal::result_status::empty) // NOLINT(hicpp-member-init)
        {
            swap(*this, other);
        }
        ~result()
        {
            if (this->status == internal::result_status::error)
                this->error.~Err();
        }

        result& operator=(const result&) = delete;
        /// @brief Move-assignable.
        result& operator=(result&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        friend constexpr void swap(result& a, result& b) noexcept
        {
            using std::swap;

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (&a != &b && (a.status == internal::result_status::error || b.status == internal::result_status::error))
                std::swap_ranges(_asr(byte*, &a.error), _asr(byte*, &a.error) + sizeof(internal::result_storage_type<Err>), _asr(byte*, &b.error));
            swap(a.status, b.status);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        friend struct sys::internal::result_b<result, void, Err>;
        friend struct sys::internal::result_b_err<result, void, Err>;
    };

    /// @brief Awaiter to enable short-circuiting, akin to rustlang's `operator?`.
    template <typename T, typename Err>
    struct result_awaiter final
    {
        /// @cond

        [[nodiscard]] _inline_always constexpr bool await_ready() const noexcept { return res; }
        template <typename Promise>
        _inline_always constexpr void await_suspend(std::coroutine_handle<Promise> parent)
        {
            if constexpr (!std::is_same_v<Err, void>)
                parent.promise().return_value(res.err());
            else if constexpr (requires { parent.promise().return_void(); })
                parent.promise().return_void();
            else
                parent.promise().return_value(nullptr);

            if constexpr (requires { parent.promise().continuation.resume(); })
                parent.promise().continuation.resume();
        }
        _inline_always constexpr T await_resume() const noexcept(std::is_same_v<T, void>) { return res.move(); }

        /// @endcond

        friend struct sys::result<T, Err>;
    private:
        result<T, Err>& res; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    };
} // namespace sys
