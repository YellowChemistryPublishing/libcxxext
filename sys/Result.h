#pragma once

/// @file

#include <algorithm>
#include <cassert>
#include <concepts>
#include <coroutine>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include <LanguageSupport.h>
#include <RecurringTemplate.h>
#include <inline/Integer.inl>
#include <meta/Type.h>

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
    /// @note Pass `byval`.
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
    using result_storage_type = std::conditional_t<!std::is_lvalue_reference_v<T>, std::remove_cv_t<T>, std::remove_reference_t<T>*>;

    /// @internal
    /// @brief Shared functionality for result types.
    template <template <typename T, typename Err> class Result, typename T, typename Err>
    struct result_b : recurring_template<Result<T, Err>>
    {
    protected:
        result_b() noexcept = default;
    public:
        static_assert(!meta::template_type<std::remove_cvref_t<T>>::template is_from<Result>(), "No monkey business with result types holding other result types!");
        static_assert(!meta::template_type<std::remove_cvref_t<Err>>::template is_from<Result>(), "No monkey business with result types holding other result types!");

        /// @brief Whether the result is good.
        constexpr explicit operator bool() const noexcept { return this->downcast().status == result_status::ok; }
        /// @brief Whether the result is bad.
        /// @note We abuse operator overloading to make it so that both `operator bool()` and `operator!()` are false for an empty result!
        constexpr bool operator!() const noexcept { return this->downcast().status == result_status::error; }

        /// @brief Apply `func(*this)` and return its output.
        [[nodiscard]] constexpr auto transform(auto&& func) noexcept(noexcept(func(std::move(this->downcast()))))
        requires requires { func(std::declval<Result<T, Err>&&>()); }
        {
            return func(std::move(this->downcast()));
        }

        /// @brief Returns an awaiter to enable short-circuiting, akin to rustlang's `operator?`.
        _inline_always result_awaiter<T, Err> operator co_await() { return result_awaiter<T, Err>(this->downcast()); }
    };
    /// @internal
    /// @brief Shared functionality for result types that are good.
    template <template <typename T, typename Err> class Result, typename T, typename Err>
    struct result_b_ok : recurring_template<Result<T, Err>>
    {
    protected:
        result_b_ok() noexcept = default;

        /// @internal
        /// @brief Constructs a result with a value.
        template <typename With>
        constexpr void ctor_ok(With&& val) noexcept(std::is_lvalue_reference_v<T> || noexcept(T(std::forward<With>(val))))
        requires requires {
            requires !std::is_lvalue_reference_v<T> || std::is_lvalue_reference_v<With&&>;
            requires std::is_lvalue_reference_v<T> || requires { T(std::forward<With>(val)); };
        }
        {
            if constexpr (std::is_lvalue_reference_v<T>)
                this->downcast().value = std::addressof(val);
            else
                std::construct_at(std::addressof(this->downcast().value), std::forward<With>(val));
            this->downcast().status = internal::result_status::ok;
        }
        /// @internal
        /// @brief Inplace constructs a result with a value.
        template <typename... Args>
        constexpr void ctor_ok(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...)))
        requires requires {
            requires !std::is_lvalue_reference_v<T>;
            T(std::forward<Args>(args)...);
        }
        {
            std::construct_at(std::addressof(this->downcast().value), std::forward<Args>(args)...);
            this->downcast().status = internal::result_status::ok;
        }

        [[nodiscard]] constexpr T move(unsafe) noexcept(std::is_lvalue_reference_v<T> || noexcept(T(std::declval<T&&>())))
        {
            if constexpr (std::is_lvalue_reference_v<T>)
            {
                T ret = *this->downcast().value;
                this->downcast().status = result_status::empty;
                return ret;
            }
            else
            {
                T ret = std::move(this->downcast().value);
                std::destroy_at(std::addressof(this->downcast().value));
                this->downcast().status = result_status::empty;
                return ret;
            }
        }
    public:
        /// @brief Takes the value if the result is good.
        /// @pre `*this == true`
        [[nodiscard]] constexpr T move() noexcept(noexcept(this->move(unsafe())))
        {
            _contract_assert(this->downcast().status == result_status::ok, "Taking value for a bad result!");
            return this->move(unsafe());
        }
        /// @brief `this->move()` if the result is good, otherwise `other`.
        template <typename With>
        [[nodiscard]] constexpr T move_or(With&& other) noexcept(noexcept(this->move(unsafe())) && noexcept(T(std::forward<With>(other))))
        requires (!std::is_lvalue_reference_v<T> || std::is_lvalue_reference_v<With &&>)
        {
            _retif(T(std::forward<With>(other)), this->downcast().status != result_status::ok);
            return this->move(unsafe());
        }
    };
    /// @internal
    /// @brief Shared functionality for result types that are bad.
    template <template <typename T, typename Err> class Result, typename T, typename Err>
    struct result_b_err : recurring_template<Result<T, Err>>
    {
    protected:
        result_b_err() noexcept = default;

        /// @internal
        /// @brief Inplace constructs a result with an error.
        template <typename... Args>
        constexpr void ctor_err(Args&&... args) noexcept(noexcept(Err(std::forward<Args>(args)...)))
        requires requires { Err(std::forward<Args>(args)...); }
        {
            std::construct_at(std::addressof(this->downcast().error), std::forward<Args>(args)...);
            this->downcast().status = internal::result_status::error;
        }

        [[nodiscard]] constexpr Err err(unsafe) noexcept(noexcept(Err(std::move(this->downcast().error))))
        {
            Err ret = std::move(this->downcast().error);
            std::destroy_at(std::addressof(this->downcast().error));
            this->downcast().status = internal::result_status::empty;
            return ret;
        }
    public:
        /// @brief Take the error of a bad result.
        /// @pre `*this == false`
        [[nodiscard]] constexpr Err err() noexcept(noexcept(Err(this->err(unsafe()))))
        {
            _contract_assert(this->downcast().status == result_status::error, "Taking error for a good or empty result!");
            return this->err(unsafe());
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
    concept IResultStorable = requires {
        requires !std::same_as<std::remove_cvref_t<T>, std::nullptr_t>;
        requires !std::same_as<std::remove_cvref_t<T>, error_tag>;
        requires std::is_lvalue_reference_v<T> || std::same_as<T, void> || (std::same_as<T, std::remove_cvref_t<T>> && !std::is_array_v<T> && std::is_nothrow_destructible_v<T>);
    };

    /// @brief A monadic type that can hold either a value or an error.
    /// @details Like the one in rustlang!
    /// @note Pass `byref`.
    template <IResultStorable T, IResultStorable Err = void>
    requires (!std::is_lvalue_reference_v<Err>) /* Intentionally don't support reference errors--doesn't really make sense. */
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
        template <typename With>
        constexpr result(With&& val) noexcept(noexcept(this->ctor_ok(std::forward<With>(val))))
        requires requires {
            requires !std::same_as<With&&, result&&>;
            requires std::same_as<T, Err> || !std::same_as<std::remove_cvref_t<With>, Err>;
            requires requires { this->ctor_ok(std::forward<With>(val)); };
        }
        {
            this->ctor_ok(std::forward<With>(val));
        }
        /// @brief Inplace constructs a result with a value.
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(this->ctor_ok(std::forward<Args>(args)...)))
        requires requires { this->ctor_ok(std::forward<Args>(args)...); }
        {
            this->ctor_ok(std::forward<Args>(args)...);
        }
        /// @brief Inplace constructs a result with an error.
        template <typename... Args>
        constexpr result(error_tag, Args&&... args) noexcept(noexcept(this->ctor_err(std::forward<Args>(args)...)))
        requires requires { this->ctor_err(std::forward<Args>(args)...); }
        {
            this->ctor_err(std::forward<Args>(args)...);
        }
        /// @brief Constructs a result with an error.
        /// @note Participates in overload resolution only if `err` cannot construct a `T`.
        template <typename With>
        constexpr result(With&& err) noexcept(noexcept(result(error_tag(), std::forward<With>(err))))
        requires requires {
            requires !std::same_as<With&&, result&&>;
            requires (!std::same_as<T, Err> && !std::same_as<T, With &&> && std::same_as<std::remove_cvref_t<With>, Err>) || !requires { T(std::forward<With>(err)); };
            requires !std::is_lvalue_reference_v<T> || std::is_lvalue_reference_v<With&&>;
            requires requires { this->ctor_err(std::forward<With>(err)); };
        }
            : result(error_tag(), std::forward<With>(err))
        { }
        /// @brief Inplace constructs a result with an error.
        /// @see `sys::result<T, Err>::result(Args&&...)`
        /// @note Participates in overload resolution only if `args...` cannot construct a `T`.
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(result(error_tag(), std::forward<Args>(args)...)))
        requires requires {
            requires !requires { T(std::forward<Args>(args)...); };
            requires requires { this->ctor_err(std::forward<Args>(args)...); };
        }
            : result(error_tag(), std::forward<Args>(args)...)
        { }

        // NOLINTEND(hicpp-explicit-conversions)

        constexpr result(const result&) = delete;
        constexpr result(result&& other) noexcept((std::is_lvalue_reference_v<T> || std::is_nothrow_move_constructible_v<T>) && std::is_nothrow_move_constructible_v<Err>)
        {
            switch (other.status)
            {
            [[likely]] case internal::result_status::ok:
                this->ctor_ok(other.move(unsafe()));
                break;
            [[unlikely]] case internal::result_status::error:
                this->ctor_err(other.err(unsafe()));
                break;
            [[unlikely]] case internal::result_status::empty:
            [[unlikely]] default:
                other.status = internal::result_status::empty;
            }
        }
        constexpr ~result()
        {
            switch (this->status)
            {
            [[likely]] case internal::result_status::ok:
                if constexpr (!std::is_lvalue_reference_v<T>)
                    std::destroy_at(std::addressof(this->value));
                break;
            [[unlikely]] case internal::result_status::error:
                std::destroy_at(std::addressof(this->error));
                break;
            [[unlikely]] case internal::result_status::empty:
            [[unlikely]] default:;
            }
        }

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept(std::is_nothrow_move_constructible_v<result>)
        {
            _retif(*this, this == std::addressof(other));
            std::destroy_at(this);
            std::construct_at(this, std::move(other));
            return *this;
        }

        friend struct sys::internal::result_b<result, T, Err>;
        friend struct sys::internal::result_b_ok<result, T, Err>;
        friend struct sys::internal::result_b_err<result, T, Err>;
    };

    /// @brief Specialization of `sys::result<...>` with a unit error type.
    /// @details For a result with a single possible error state, iow. a valueless error.
    template <IResultStorable T>
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
        // NOLINTBEGIN(hicpp-explicit-conversions, hicpp-member-init)

        /// @brief Constructs a result with a value.
        template <typename With>
        constexpr result(With&& val) noexcept(noexcept(this->ctor_ok(std::forward<With>(val))))
        requires requires {
            requires !std::same_as<With&&, result&&>;
            requires requires { this->ctor_ok(std::forward<With>(val)); };
        }
        {
            this->ctor_ok(std::forward<With>(val));
        }
        /// @brief Inplace constructs a result with a value.
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(this->ctor_ok(std::forward<Args>(args)...)))
        requires requires { this->ctor_ok(std::forward<Args>(args)...); }
        {
            this->ctor_ok(std::forward<Args>(args)...);
        }
        /// @brief Construct an error result.
        constexpr result(std::nullptr_t) noexcept : status(internal::result_status::error) { }
        constexpr result(const result&) = delete;
        constexpr result(result&& other) noexcept((std::is_lvalue_reference_v<T> || std::is_nothrow_move_constructible_v<T>))
        {
            switch (other.status)
            {
            [[likely]] case internal::result_status::ok:
                this->ctor_ok(other.move(unsafe()));
                break;
            [[unlikely]] case internal::result_status::error:
                this->status = internal::result_status::error;
                [[fallthrough]];
            [[unlikely]] case internal::result_status::empty:
            [[unlikely]] default:
                other.status = internal::result_status::empty;
            }
        }
        ~result()
        {
            if constexpr (!std::is_lvalue_reference_v<T>)
                if (this->status == internal::result_status::ok) [[likely]]
                    std::destroy_at(std::addressof(this->value));
        }

        // NOLINTEND(hicpp-explicit-conversions, hicpp-member-init)

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept(std::is_nothrow_move_constructible_v<result>)
        {
            _retif(*this, this == std::addressof(other));
            std::destroy_at(this);
            std::construct_at(this, std::move(other));
            return *this;
        }

        friend struct sys::internal::result_b<result, T, void>;
        friend struct sys::internal::result_b_ok<result, T, void>;
    };

    /// @brief Specialization of `sys::result<...>` that holds no value if ok.
    /// @details For a result with a single possible success state.
    template <typename Err>
    requires (!std::is_lvalue_reference_v<Err>)
    struct [[nodiscard]] result<void, Err> final : internal::result_b<result, void, Err>, internal::result_b_err<result, void, Err>
    {
    private:
        union
        {
            byte _;
            internal::result_storage_type<Err> error;
        };
        internal::result_status status = internal::result_status::empty;
    public:
        // NOLINTBEGIN(hicpp-explicit-conversions, hicpp-member-init)

        /// @brief Construct a success result.
        constexpr result() noexcept : status(internal::result_status::ok) { }
        /// @brief Inplace constructs a result with an error.
        template <typename... Args>
        constexpr result(error_tag, Args&&... args) noexcept(noexcept(this->ctor_err(std::forward<Args>(args)...)))
        requires requires { this->ctor_err(std::forward<Args>(args)...); }
        {
            this->ctor_err(std::forward<Args>(args)...);
        }
        /// @brief Constructs a result with an error.
        template <typename With>
        constexpr result(With&& err) noexcept(noexcept(result(error_tag(), std::forward<With>(err))))
        requires requires {
            requires !std::same_as<With&&, result&&>;
            requires requires { this->ctor_err(std::forward<With>(err)); };
        }
            : result(error_tag(), std::forward<With>(err))
        { }
        /// @brief Inplace constructs a result with an error.
        /// @see `sys::result<T, Err>::result(Args&&...)`
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(result(error_tag(), std::forward<Args>(args)...)))
        requires requires {
            requires sizeof...(Args) != 1uz;
            requires requires { this->ctor_err(std::forward<Args>(args)...); };
        }
            : result(error_tag(), std::forward<Args>(args)...)
        { }
        constexpr result(const result&) = delete;
        constexpr result(result&& other) noexcept(std::is_nothrow_move_constructible_v<Err>) : status(other.status)
        {
            switch (other.status)
            {
            [[unlikely]] case internal::result_status::error:
                this->ctor_err(other.err(unsafe()));
                break;
            [[likely]] case internal::result_status::ok:
                this->status = internal::result_status::ok;
                [[fallthrough]];
            [[unlikely]] case internal::result_status::empty:
            [[unlikely]] default:
                other.status = internal::result_status::empty;
            }
        }
        ~result()
        {
            if (this->status == internal::result_status::error) [[unlikely]]
                std::destroy_at(std::addressof(this->error));
        }

        // NOLINTEND(hicpp-explicit-conversions, hicpp-member-init)

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept(std::is_nothrow_move_constructible_v<result>)
        {
            _retif(*this, this == &other);
            std::destroy_at(this);
            std::construct_at(this, std::move(other));
            return *this;
        }

        friend struct sys::internal::result_b<result, void, Err>;
        friend struct sys::internal::result_b_err<result, void, Err>;
    };

    /// @brief Specialization of `sys::result<...>` representing a boolean result.
    /// @details A result valueless in both `T` and `Err`, iow. a `bool` with more explicit semantics.
    template <>
    struct [[nodiscard]] result<void, void> final : internal::result_b<result, void, void>
    {
    private:
        internal::result_status status = internal::result_status::empty;
    public:
        // NOLINTBEGIN(hicpp-explicit-conversions, hicpp-member-init)

        /// @brief Construct a success result.
        constexpr result() noexcept : status(internal::result_status::ok) { }
        /// @brief Construct an error result.
        constexpr result(std::nullptr_t) noexcept : status(internal::result_status::error) { }
        constexpr result(const result&) = delete;
        constexpr result(result&& other) noexcept : status(other.status) { other.status = internal::result_status::empty; }
        constexpr ~result() = default;

        // NOLINTEND(hicpp-explicit-conversions, hicpp-member-init)

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept(std::is_nothrow_move_constructible_v<result>)
        {
            swap(*this, other);
            return *this;
        }

        friend void swap(result& a, result& b) noexcept { std::swap(a.status, b.status); }

        friend struct sys::internal::result_b<result, void, void>;
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
