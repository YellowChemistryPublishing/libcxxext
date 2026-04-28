#pragma once

/// @file

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include <AlignedStorage.h>
#include <CompilerWarnings.h>
#include <LanguageSupport.h>
#include <RecurringTemplate.h>
#include <meta/NamedRequirements.h>
#include <meta/Nullable.h>
#include <meta/Type.h>

// NOLINTBEGIN(bugprone-macro-parentheses)
/// @def _res_movret(out_decl, res_xval)
/// @ingroup sys
/// @brief Initialize `out_decl` with `res_xval`, early-returning if `res_xval` is an error.
#define _res_movret(out_decl, res_xval)              \
    _nowarn_begin_one_clang(_clwarn_clang_consumed); \
    auto _ppcat(_result, __LINE__) = res_xval;       \
    if (!_ppcat(_result, __LINE__))                  \
    {                                                \
        return [](auto& res) -> auto                 \
        {                                            \
            if constexpr (requires { res.err(); })   \
                return res.err();                    \
            else                                     \
                return nullptr;                      \
        }(_ppcat(_result, __LINE__));                \
    }                                                \
    out_decl = _ppcat(_result, __LINE__).move();     \
    _nowarn_end_clang();
/// @def _res_movcoret(out_decl, res_xval)
/// @ingroup sys
/// @brief Initialize `out_decl` with `res_xval`, early-coroutine-returning if `res_xval` is an error.
#define _res_movcoret(out_decl, res_xval)            \
    _nowarn_begin_one_clang(_clwarn_clang_consumed); \
    auto _ppcat(_result, __LINE__) = res_xval;       \
    if (!_ppcat(_result, __LINE__))                  \
    {                                                \
        co_return [](auto& res) -> auto              \
        {                                            \
            if constexpr (requires { res.err(); })   \
                return res.err();                    \
            else                                     \
                return nullptr;                      \
        }(_ppcat(_result, __LINE__));                \
    }                                                \
    out_decl = _ppcat(_result, __LINE__).move();     \
    _nowarn_end_clang();
// NOLINTEND(bugprone-macro-parentheses)

namespace sys::internal
{
    /// @internal
    /// @ingroup sys_internal
    /// @brief Status of a result.
    /// @note Pass `byval`.
    enum class result_status : byte
    {
        ok,
        error,
        empty
    };

    /// @internal
    /// @ingroup sys_internal
    /// @brief Type to use in result storage to represent `T`.
    template <typename T>
    using result_storage_type = std::conditional_t<!meta::type<T>::is_lvalue(), std::remove_cvref_t<T>, std::remove_reference_t<T>*>;

    /// @internal
    /// @ingroup sys_internal
    /// @brief Shared functionality for result types.
    template <template <typename, typename> class Result, typename T, typename Err>
    struct result_b : public recurring_template<Result<T, Err>>
    {
    protected:
        result_b() noexcept = default;
    public:
        static_assert(!meta::type<std::remove_cvref_t<T>>::template is_from<Result>(), "No monkey business with result types holding other result types!");
        static_assert(!meta::type<std::remove_cvref_t<Err>>::template is_from<Result>(), "No monkey business with result types holding other result types!");

        using value_type = T;
        using err_type = Err;
    };
    /// @internal
    /// @ingroup sys_internal
    /// @brief Shared functionality for result types that have a bad value.
    template <template <typename, typename> class Result, typename T, typename Err>
    struct result_b_err : public recurring_template<Result<T, Err>>
    {
    protected:
        result_b_err() noexcept = default;

        /// @internal
        /// @brief Constructs a result with an error.
        template <typename With>
        constexpr void ctor_err(With&& val) noexcept(meta::type<Err>::is_lvalue() || noexcept(Err(std::forward<With>(val))))
        requires requires {
            requires !meta::type<Err>::is_lvalue() || meta::type<With&&>::is_lvalue();
            requires meta::type<Err>::is_lvalue() || requires { Err(std::forward<With>(val)); };
        }
        {
            if constexpr (meta::type<Err>::is_lvalue())
                *this->downcast().storage.template data<result_storage_type<Err>>() = std::addressof(val);
            else
                std::construct_at(this->downcast().storage.template data<result_storage_type<Err>>(), std::forward<With>(val));
            this->downcast().status = result_status::error;
        }
        /// @internal
        /// @brief Inplace constructs a result with an error.
        template <typename... Args>
        constexpr void ctor_err(Args&&... args) noexcept(noexcept(Err(std::forward<Args>(args)...)))
        requires requires { Err(std::forward<Args>(args)...); }
        {
            std::construct_at(this->downcast().storage.template data<result_storage_type<Err>>(), std::forward<Args>(args)...);
            this->downcast().status = result_status::error;
        }

        /// @internal
        /// @brief Takes the error of a bad result.
        /// @warning `unsafe` because of unchecked tagged union access.
        [[nodiscard]] constexpr Err err(decltype(unsafe)) noexcept(meta::type<Err>::is_lvalue() || INothrowMoveConstructible<Err>)
        {
            if constexpr (meta::type<Err>::is_lvalue())
            {
                Err ret = **this->downcast().storage.template data<result_storage_type<Err>>();
                this->downcast().status = result_status::empty;
                return ret;
            }
            else
            {
                Err ret = std::move(*this->downcast().storage.template data<result_storage_type<Err>>());
                this->downcast().status = result_status::empty;
                std::destroy_at(this->downcast().storage.template data<result_storage_type<Err>>());
                return ret;
            }
        }

        /// @internal
        /// @brief Convert to a result with a single error state.
        [[nodiscard]] constexpr explicit operator Result<T, void>() && noexcept(std::same_as<T, void> || meta::type<T>::is_lvalue() || requires {
            requires !std::same_as<T, void>;
            requires INothrowMoveConstructible<T>;
        })
        {
            switch (this->downcast().status)
            {
            [[likely]] case result_status::error:
                return Result<T, void>(nullptr);
            [[likely]] case result_status::ok:
                if constexpr (std::same_as<T, void>)
                    return Result<T, void>();
                else
                    return Result<T, void>(this->downcast().move(unsafe));
            [[unlikely]] case result_status::empty:
            [[unlikely]] default:
                return Result<T, void>(unsafe);
            }
        }

        friend Result<T, Err>; // Because MSVC is a moron.
    };
    /// @internal
    /// @ingroup sys_internal
    /// @brief Shared functionality for result types that have a good value.
    template <template <typename, typename> class Result, typename T, typename Err>
    struct result_b_ok : public recurring_template<Result<T, Err>>
    {
    protected:
        result_b_ok() noexcept = default;

        /// @internal
        /// @brief Constructs a result with a value.
        template <typename With>
        constexpr void ctor_ok(With&& val) noexcept(meta::type<T>::is_lvalue() || noexcept(T(std::forward<With>(val))))
        requires requires {
            requires !meta::type<T>::is_lvalue() || meta::type<With&&>::is_lvalue();
            requires meta::type<T>::is_lvalue() || requires { T(std::forward<With>(val)); };
        }
        {
            if constexpr (meta::type<T>::is_lvalue())
                *this->downcast().storage.template data<result_storage_type<T>>() = std::addressof(val);
            else
                std::construct_at(this->downcast().storage.template data<result_storage_type<T>>(), std::forward<With>(val));
            this->downcast().status = result_status::ok;
        }
        /// @internal
        /// @brief Inplace constructs a result with a value.
        template <typename... Args>
        constexpr void ctor_ok(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...)))
        requires requires {
            requires !meta::type<T>::is_lvalue();
            T(std::forward<Args>(args)...);
        }
        {
            std::construct_at(this->downcast().storage.template data<result_storage_type<T>>(), std::forward<Args>(args)...);
            this->downcast().status = result_status::ok;
        }

        /// @internal
        /// @brief Takes the value of a good result.
        /// @warning `unsafe` because of unchecked tagged union access.
        [[nodiscard]] constexpr T move(decltype(unsafe)) noexcept(meta::type<T>::is_lvalue() || INothrowMoveConstructible<T>)
        {
            if constexpr (meta::type<T>::is_lvalue())
            {
                T ret = **this->downcast().storage.template data<result_storage_type<T>>();
                this->downcast().status = result_status::empty;
                return ret;
            }
            else
            {
                T ret = std::move(*this->downcast().storage.template data<result_storage_type<T>>());
                this->downcast().status = result_status::empty;
                std::destroy_at(this->downcast().storage.template data<result_storage_type<T>>());
                return ret;
            }
        }
    public:
        template <template <typename, typename> class, typename, typename>
        friend struct sys::internal::result_b_err;
    };
} // namespace sys::internal

namespace sys
{
    /// @ingroup sys
    /// @brief Tag value to force construction of a bad result.
    constexpr struct
    {
    } error_tag;

    /// @ingroup sys
    /// @brief Concept for types that can be stored in a result.
    template <typename T>
    concept IResultStorable = !requires {
        { sizeof(T) } -> std::same_as<size_t>; /* Allow declaration with incomplete type. */
    } || requires {
        requires !std::same_as<std::remove_cvref_t<T>, std::nullptr_t>;
        requires !std::same_as<std::remove_cvref_t<T>, decltype(error_tag)>;
        requires meta::type<T>::is_lvalue() || std::same_as<T, void> ||
            (meta::type<T>::is_unqualified() && !meta::type<T>::is_array() && (IMoveConstructible<T>) && (IMoveAssignable<T>) && (INothrowDestructible<T>));
    };

    /// @ingroup sys
    /// @brief A monadic type that can hold either a value or an error.
    /// @details Like the one in rustlang!
    /// @note Pass `byref`.
    template <IResultStorable T, IResultStorable Err = void>
    class [[nodiscard, clang::consumable(unconsumed)]] result final : public internal::result_b<result, T, Err>,
                                                                      public internal::result_b_ok<result, T, Err>,
                                                                      public internal::result_b_err<result, T, Err>
    {
        aligned_storage<internal::result_storage_type<T>, internal::result_storage_type<Err>> storage;
        internal::result_status status = internal::result_status::empty;

        using internal::result_b_ok<result, T, Err>::move;
        using internal::result_b_err<result, T, Err>::err;
    public:
        // NOLINTBEGIN(hicpp-explicit-conversions)

        /// @brief Constructs a result with a value.
        template <typename With>
        constexpr result(With&& val) noexcept(meta::type<T>::is_lvalue() || noexcept(T(std::forward<With>(val))))
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
        constexpr result(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...)))
        requires requires { this->ctor_ok(std::forward<Args>(args)...); }
        {
            this->ctor_ok(std::forward<Args>(args)...);
        }
        /// @brief Inplace constructs a result with an error.
        template <typename... Args>
        constexpr result(decltype(error_tag), Args&&... args) noexcept(noexcept(Err(std::forward<Args>(args)...)))
        requires requires { this->ctor_err(std::forward<Args>(args)...); }
        {
            this->ctor_err(std::forward<Args>(args)...);
        }
        /// @brief Constructs a result with an error.
        /// @note Participates in overload resolution only if `err` cannot construct a `T`.
        template <typename With>
        constexpr result(With&& err) noexcept(noexcept(Err(std::forward<With>(err))))
        requires requires {
            requires !std::same_as<With&&, result&&>;
            requires (!std::same_as<T, Err> && !std::same_as<T, With &&> && std::same_as<std::remove_cvref_t<With>, Err>) || !requires { T(std::forward<With>(err)); };
            requires !meta::type<T>::is_lvalue() || meta::type<With&&>::is_lvalue();
            requires requires { this->ctor_err(std::forward<With>(err)); };
        }
            : result(error_tag, std::forward<With>(err))
        { }
        /// @brief Inplace constructs a result with an error.
        /// @see `sys::result<T, Err>::result(Args&&...)`
        /// @note Participates in overload resolution only if `args...` cannot construct a `T`.
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(Err(std::forward<Args>(args)...)))
        requires requires {
            requires !requires { T(std::forward<Args>(args)...); };
            requires requires { this->ctor_err(std::forward<Args>(args)...); };
        }
            : result(error_tag, std::forward<Args>(args)...)
        { }

        // NOLINTEND(hicpp-explicit-conversions)

        constexpr result(const result&) = delete;
        constexpr result([[clang::return_typestate(unknown)]] result&& other) noexcept((meta::type<T>::is_lvalue() || INothrowMoveConstructible<T>) &&
                                                                                       (meta::type<Err>::is_lvalue() || INothrowMoveConstructible<Err>))
        {
            switch (other.status)
            {
            case internal::result_status::ok: this->ctor_ok(other.move(unsafe)); break;
            case internal::result_status::error: this->ctor_err(other.err(unsafe)); break;
            default:;
            }
            other.status = internal::result_status::empty;
        }
        [[clang::callable_when("consumed", "unknown")]] constexpr ~result()
        {
            switch (this->status)
            {
            case internal::result_status::ok:
                if constexpr (!meta::type<T>::is_lvalue())
                    std::destroy_at(this->storage.template data<internal::result_storage_type<T>>());
                break;
            case internal::result_status::error:
                if constexpr (!meta::type<Err>::is_lvalue())
                    std::destroy_at(this->storage.template data<internal::result_storage_type<Err>>());
                break;
            default:;
            }
        }

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept((meta::type<T>::is_lvalue() || (INothrowSwappable<T> && INothrowMoveConstructible<T>)) &&
                                                   (meta::type<Err>::is_lvalue() || (INothrowSwappable<Err> && INothrowMoveConstructible<Err>)))
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Whether the result is good.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(consumed)]] constexpr explicit operator bool() const noexcept
        {
            return this->status == internal::result_status::ok;
        }
        /// @brief Whether the result is bad.
        /// @note We abuse operator overloading to make it so that both `operator bool()` and `operator!()` are `false` for an empty result!
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(consumed)]] constexpr bool operator!() const noexcept
        {
            return this->status == internal::result_status::error;
        }

        /// @brief Convert to a result with a single error state.
        [[nodiscard, clang::set_typestate(unknown)]] constexpr explicit operator result<T, void>() && noexcept(std::same_as<T, void> || meta::type<T>::is_lvalue() ||
                                                                                                               INothrowMoveConstructible<T>)
        {
            return _as(std::move(_as(*this, internal::result_b_err<result, T, Err>&)), result<T, void>);
        }

        /// @brief Takes the value if the result has a good value.
        /// @pre `*this == true`
        [[nodiscard, clang::callable_when("consumed"), clang::set_typestate(unknown)]] constexpr T move() noexcept(meta::type<T>::is_lvalue() || INothrowMoveConstructible<T>)
        {
            _contract_assert(this->status == internal::result_status::ok, "Taking value for a bad result!"); // LCOV_EXCL_LINE
            return this->move(unsafe);
        }
        /// @brief `this->move()` if the result is good, otherwise `other`.
        template <typename With>
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr T move_or(With&& other) noexcept(
            (meta::type<T>::is_lvalue() || INothrowMoveConstructible<T>) && noexcept(T(std::forward<With>(other))))
        requires (!meta::type<T>::is_lvalue() || meta::type<With &&>::is_lvalue())
        {
            _retif(T(std::forward<With>(other)), this->status != internal::result_status::ok);
            return this->move(unsafe);
        }
        /// @brief Takes the value if the result has a good value.
        /// @pre `*this == true`
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr T expect() noexcept(meta::type<T>::is_lvalue() ||
                                                                                                                                   INothrowMoveConstructible<T>)
        {
            _contract_assert(this->status == internal::result_status::ok, "Taking value for a bad result!"); // LCOV_EXCL_LINE
            return this->move(unsafe);
        }

        /// @brief Take the error of a bad result.
        /// @pre `*this == false`
        [[nodiscard, clang::callable_when("consumed"), clang::set_typestate(unknown)]] constexpr Err err() noexcept(meta::type<Err>::is_lvalue() || INothrowMoveConstructible<Err>)
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_LINE
            return this->err(unsafe);
        }
        /// @brief Take the error of a bad result.
        /// @pre `*this == false`
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr Err expect_err() noexcept(meta::type<Err>::is_lvalue() ||
                                                                                                                                         INothrowMoveConstructible<Err>)
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_LINE
            return this->err(unsafe);
        }

        /// @brief Apply `func(*this)` and return its output.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr auto transform(auto&& func) && noexcept(
            noexcept(func(std::move(*this))))
        requires requires { func(std::move(*this)); }
        {
            return func(std::move(*this));
        }

        friend void swap([[clang::param_typestate(unconsumed), clang::return_typestate(unconsumed)]] result& a,
                         [[clang::param_typestate(unconsumed),
                           clang::return_typestate(unconsumed)]] result& b) noexcept((meta::type<T>::is_lvalue() || (INothrowSwappable<T> && INothrowMoveConstructible<T>)) &&
                                                                                     (meta::type<Err>::is_lvalue() || (INothrowSwappable<Err> && INothrowMoveConstructible<Err>)))
        {
            using std::swap;

            constexpr auto swapOkWithErr = [](result& ok_res, result& err_res) -> void
            {
                if constexpr (sizeof(internal::result_storage_type<Err>) <= sizeof(internal::result_storage_type<T>))
                {
                    Err err = err_res.err(unsafe);
                    err_res.ctor_ok(ok_res.move(unsafe));
                    ok_res.ctor_err(std::move(err));
                }
                else
                {
                    T val = ok_res.move(unsafe);
                    ok_res.ctor_err(err_res.err(unsafe));
                    err_res.ctor_ok(std::move(val));
                }
            };

            switch (a.status)
            {
            case internal::result_status::ok:
                switch (b.status)
                {
                case internal::result_status::ok:
                    swap(*a.storage.template data<internal::result_storage_type<T>>(), *b.storage.template data<internal::result_storage_type<T>>());
                    break;
                case internal::result_status::error: swapOkWithErr(a, b); break;
                default: b.ctor_ok(a.move(unsafe));
                }
                break;
            case internal::result_status::error:
                switch (b.status)
                {
                case internal::result_status::ok: swapOkWithErr(b, a); break;
                case internal::result_status::error:
                    swap(*a.storage.template data<internal::result_storage_type<Err>>(), *b.storage.template data<internal::result_storage_type<Err>>());
                    break;
                default: b.ctor_err(a.err(unsafe));
                }
                break;
            default:
                switch (b.status)
                {
                case internal::result_status::ok: a.ctor_ok(b.move(unsafe)); break;
                case internal::result_status::error: a.ctor_err(b.err(unsafe)); break;
                default:;
                }
            }
        }

        friend struct sys::internal::result_b<result, T, Err>;
        friend struct sys::internal::result_b_ok<result, T, Err>;
        friend struct sys::internal::result_b_err<result, T, Err>;
    };

    /// @ingroup sys
    /// @brief Specialization of `sys::result<...>` representing a boolean result.
    /// @details A result valueless in both `T` and `Err`, iow. a `bool` with more explicit semantics.
    template <>
    class [[nodiscard, clang::consumable(unconsumed)]] result<void, void> final : public internal::result_b<result, void, void>
    {
        internal::result_status status = internal::result_status::empty;

        constexpr explicit result(decltype(unsafe)) noexcept { };
    public:
        // NOLINTBEGIN(hicpp-explicit-conversions, hicpp-member-init)

        /// @brief Construct a success result.
        [[clang::return_typestate(unconsumed)]] constexpr result() noexcept : status(internal::result_status::ok) { }
        /// @brief Construct an error result.
        constexpr result(std::nullptr_t) noexcept : status(internal::result_status::error) { }
        constexpr result(const result&) = delete;
        constexpr result([[clang::return_typestate(unknown)]] result&& other) noexcept : status(other.status) { other.status = internal::result_status::empty; }
        [[clang::callable_when("consumed")]] constexpr ~result() = default;

        // NOLINTEND(hicpp-explicit-conversions, hicpp-member-init)

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Whether the result is good.
        [[nodiscard, clang::set_typestate(consumed)]] constexpr explicit operator bool() const noexcept { return this->status == internal::result_status::ok; }
        /// @brief Whether the result is bad.
        /// @note We abuse operator overloading to make it so that both `operator bool()` and `operator!()` are `false` for an empty result!
        [[nodiscard, clang::set_typestate(consumed)]] constexpr bool operator!() const noexcept { return this->status == internal::result_status::error; }

        /// @brief Expects the result to be good.
        /// @pre `*this == true`
        [[clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] void expect() const noexcept
        {
            _contract_assert(this->status == internal::result_status::ok, "Taking value for a bad or empty result!"); // LCOV_EXCL_LINE
        }
        /// @brief Expects the result to be bad.
        /// @pre `*this == false`
        [[clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] void expect_err() const noexcept
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_LINE
        }

        /// @brief Apply `func(*this)` and return its output.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr auto transform(auto&& func) && noexcept(
            noexcept(func(std::move(*this))))
        requires requires { func(std::move(*this)); }
        {
            return func(std::move(*this));
        }

        friend void swap([[clang::param_typestate(unconsumed), clang::return_typestate(unconsumed)]] result& a,
                         [[clang::param_typestate(unconsumed), clang::return_typestate(unconsumed)]] result& b) noexcept
        {
            std::swap(a.status, b.status);
        }

        friend struct sys::internal::result_b<result, void, void>;
        template <template <typename, typename> class, typename, typename>
        friend struct sys::internal::result_b_err;
    };

    /// @ingroup sys
    /// @brief Specialization of `sys::result<...>` with a unit error type.
    /// @details For a result with a single possible error state, iow. a valueless error.
    template <IResultStorable T>
    requires (!std::same_as<T, void>)
    class [[nodiscard, clang::consumable(unconsumed)]] result<T, void> final : public internal::result_b<result, T, void>, public internal::result_b_ok<result, T, void>
    {
        aligned_storage<internal::result_storage_type<T>> storage;
        internal::result_status status = internal::result_status::empty;

        using internal::result_b<result, T, void>::downcast;
        using internal::result_b_ok<result, T, void>::move;

        // To ensure well-defined conversion from any `Err` result to `void` result.
        constexpr explicit result(decltype(unsafe)) noexcept { };
    public:
        // NOLINTBEGIN(hicpp-explicit-conversions, hicpp-member-init)

        /// @brief Constructs a result with a value.
        template <typename With>
        constexpr result(With&& val) noexcept(meta::type<T>::is_lvalue() || noexcept(T(std::forward<With>(val))))
        requires requires {
            requires !std::same_as<With&&, result&&>;
            requires requires { this->ctor_ok(std::forward<With>(val)); };
        }
        {
            this->ctor_ok(std::forward<With>(val));
        }
        /// @brief Inplace constructs a result with a value.
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...)))
        requires requires { this->ctor_ok(std::forward<Args>(args)...); }
        {
            this->ctor_ok(std::forward<Args>(args)...);
        }
        /// @brief Construct an error result.
        constexpr result(std::nullptr_t) noexcept : status(internal::result_status::error) { }
        constexpr result(const result&) = delete;
        constexpr result([[clang::return_typestate(unknown)]] result&& other) noexcept(meta::type<T>::is_lvalue() || INothrowMoveConstructible<T>)
        {
            switch (other.status)
            {
            case internal::result_status::ok: this->ctor_ok(other.move(unsafe)); break;
            case internal::result_status::error: this->status = internal::result_status::error; [[fallthrough]];
            case internal::result_status::empty:
            default: other.status = internal::result_status::empty;
            }
        }
        [[clang::callable_when("consumed", "unknown")]] ~result()
        {
            if constexpr (!meta::type<T>::is_lvalue())
                if (this->status == internal::result_status::ok) [[likely]]
                    std::destroy_at(this->downcast().storage.template data<internal::result_storage_type<T>>());
        }

        // NOLINTEND(hicpp-explicit-conversions, hicpp-member-init)

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept(meta::type<T>::is_lvalue() || (INothrowSwappable<T> && INothrowMoveConstructible<T>))
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Whether the result is good.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(consumed)]] constexpr explicit operator bool() const noexcept
        {
            return this->status == internal::result_status::ok;
        }
        /// @brief Whether the result is bad.
        /// @note We abuse operator overloading to make it so that both `operator bool()` and `operator!()` are `false` for an empty result!
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(consumed)]] constexpr bool operator!() const noexcept
        {
            return this->status == internal::result_status::error;
        }

        /// @brief Takes the value if the result has a good value.
        /// @pre `*this == true`
        [[nodiscard, clang::callable_when("consumed"), clang::set_typestate(unknown)]] constexpr T move() noexcept(meta::type<T>::is_lvalue() || INothrowMoveConstructible<T>)
        {
            _contract_assert(this->status == internal::result_status::ok, "Taking value for a bad result!"); // LCOV_EXCL_LINE
            return this->move(unsafe);
        }
        /// @brief `this->move()` if the result is good, otherwise `other`.
        template <typename With>
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr T move_or(With&& other) noexcept(
            (meta::type<T>::is_lvalue() || INothrowMoveConstructible<T>) && noexcept(T(std::forward<With>(other))))
        requires (!meta::type<T>::is_lvalue() || meta::type<With &&>::is_lvalue())
        {
            _retif(T(std::forward<With>(other)), this->status != internal::result_status::ok);
            return this->move(unsafe);
        }
        /// @brief Takes the value if the result has a good value.
        /// @pre `*this == true`
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr T expect() noexcept(meta::type<T>::is_lvalue() ||
                                                                                                                                   INothrowMoveConstructible<T>)
        {
            _contract_assert(this->status == internal::result_status::ok, "Taking value for a bad result!"); // LCOV_EXCL_LINE
            return this->move(unsafe);
        }

        /// @brief Expects the result to be bad.
        /// @pre `*this == false`
        [[clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] void expect_err() const noexcept
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_LINE
        }

        /// @brief Apply `func(*this)` and return its output.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr auto transform(auto&& func) && noexcept(
            noexcept(func(std::move(*this))))
        requires requires { func(std::move(*this)); }
        {
            return func(std::move(*this));
        }

        friend void swap([[clang::param_typestate(unconsumed), clang::return_typestate(unconsumed)]] result& a,
                         [[clang::param_typestate(unconsumed), clang::return_typestate(unconsumed)]] result& b) noexcept(meta::type<T>::is_lvalue() ||
                                                                                                                         (INothrowSwappable<T> && INothrowMoveConstructible<T>))
        {
            using std::swap;
            switch (a.status)
            {
            case internal::result_status::ok:
                switch (b.status)
                {
                case internal::result_status::ok:
                    swap(*a.storage.template data<internal::result_storage_type<T>>(), *b.storage.template data<internal::result_storage_type<T>>());
                    break;
                case internal::result_status::error:
                    b.ctor_ok(a.move(unsafe));
                    a.status = internal::result_status::error;
                    break;
                default: b.ctor_ok(a.move(unsafe)); a.status = internal::result_status::empty;
                }
                break;
            case internal::result_status::error:
                switch (b.status)
                {
                case internal::result_status::ok:
                    a.ctor_ok(b.move(unsafe));
                    b.status = internal::result_status::error;
                    break;
                case internal::result_status::error: break;
                default: swap(a.status, b.status);
                }
                break;
            default:
                switch (b.status)
                {
                case internal::result_status::ok:
                    a.ctor_ok(b.move(unsafe));
                    b.status = internal::result_status::empty;
                    break;
                case internal::result_status::error: swap(a.status, b.status); break;
                default:;
                }
            }
        }

        friend struct sys::internal::result_b<result, T, void>;
        friend struct sys::internal::result_b_ok<result, T, void>;
        template <template <typename, typename> class, typename, typename>
        friend struct sys::internal::result_b_err;
    };

    /// @ingroup sys
    /// @brief Specialization of `sys::result<...>` that holds no value if ok.
    /// @details For a result with a single possible success state.
    template <typename Err>
    requires (!std::same_as<Err, void>)
    class [[nodiscard, clang::consumable(unconsumed)]] result<void, Err> final : public internal::result_b<result, void, Err>, public internal::result_b_err<result, void, Err>
    {
        aligned_storage<internal::result_storage_type<Err>> storage;
        internal::result_status status = internal::result_status::empty;

        using internal::result_b_err<result, void, Err>::err;
    public:
        // NOLINTBEGIN(hicpp-explicit-conversions, hicpp-member-init)

        /// @brief Construct a success result.
        [[clang::return_typestate(unconsumed)]] constexpr result() noexcept : status(internal::result_status::ok) { }
        /// @brief Inplace constructs a result with an error.
        template <typename... Args>
        constexpr result(decltype(error_tag), Args&&... args) noexcept(noexcept(Err(std::forward<Args>(args)...)))
        requires requires { this->ctor_err(std::forward<Args>(args)...); }
        {
            this->ctor_err(std::forward<Args>(args)...);
        }
        /// @brief Constructs a result with an error.
        template <typename With>
        constexpr result(With&& err) noexcept(noexcept(Err(std::forward<With>(err))))
        requires requires {
            requires !std::same_as<With&&, result&&>;
            requires requires { this->ctor_err(std::forward<With>(err)); };
        }
            : result(error_tag, std::forward<With>(err))
        { }
        /// @brief Inplace constructs a result with an error.
        /// @see `sys::result<T, Err>::result(Args&&...)`
        template <typename... Args>
        constexpr result(Args&&... args) noexcept(noexcept(Err(std::forward<Args>(args)...)))
        requires requires {
            requires sizeof...(Args) != 1uz;
            requires requires { this->ctor_err(std::forward<Args>(args)...); };
        }
            : result(error_tag, std::forward<Args>(args)...)
        { }
        constexpr result(const result&) = delete;
        constexpr result([[clang::return_typestate(unknown)]] result&& other) noexcept(meta::type<Err>::is_lvalue() || INothrowMoveConstructible<Err>) : status(other.status)
        {
            switch (other.status)
            {
            case internal::result_status::error: this->ctor_err(other.err(unsafe)); break;
            case internal::result_status::ok: this->status = internal::result_status::ok; [[fallthrough]];
            case internal::result_status::empty:
            default: other.status = internal::result_status::empty;
            }
        }
        [[clang::callable_when("consumed", "unknown")]] ~result()
        {
            if constexpr (!meta::type<Err>::is_lvalue())
                if (this->status == internal::result_status::error) [[unlikely]]
                    std::destroy_at(this->storage.template data<internal::result_storage_type<Err>>());
        }

        // NOLINTEND(hicpp-explicit-conversions, hicpp-member-init)

        result& operator=(const result&) = delete;
        result& operator=(result&& other) noexcept(meta::type<Err>::is_lvalue() || (INothrowSwappable<Err> && INothrowMoveConstructible<Err>))
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Whether the result is good.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(consumed)]] constexpr explicit operator bool() const noexcept
        {
            return this->status == internal::result_status::ok;
        }
        /// @brief Whether the result is bad.
        /// @note We abuse operator overloading to make it so that both `operator bool()` and `operator!()` are `false` for an empty result!
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(consumed)]] constexpr bool operator!() const noexcept
        {
            return this->status == internal::result_status::error;
        }

        /// @brief Convert to a result with a single error state.
        [[nodiscard, clang::set_typestate(unknown)]] constexpr explicit operator result<void, void>() && noexcept
        {
            return _as(std::move(_as(*this, internal::result_b_err<result, void, Err>&)), result<void, void>);
        }

        /// @brief Expects the result to be good.
        /// @pre `*this == true`
        [[clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] void expect() const noexcept
        {
            _contract_assert(this->status == internal::result_status::ok, "Taking value for a bad or empty result!"); // LCOV_EXCL_LINE
        }

        /// @brief Take the error of a bad result.
        /// @pre `*this == false`
        [[nodiscard, clang::callable_when("consumed"), clang::set_typestate(unknown)]] constexpr Err err() noexcept(meta::type<Err>::is_lvalue() || INothrowMoveConstructible<Err>)
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_LINE
            return this->err(unsafe);
        }
        /// @brief Take the error of a bad result.
        /// @pre `*this == false`
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr Err expect_err() noexcept(meta::type<Err>::is_lvalue() ||
                                                                                                                                         INothrowMoveConstructible<Err>)
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_LINE
            return this->err(unsafe);
        }

        /// @brief Apply `func(*this)` and return its output.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr auto transform(auto&& func) && noexcept(
            noexcept(func(std::move(*this))))
        requires requires { func(std::move(*this)); }
        {
            return func(std::move(*this));
        }

        friend void swap([[clang::param_typestate(unconsumed), clang::return_typestate(unconsumed)]] result& a,
                         [[clang::param_typestate(unconsumed), clang::return_typestate(unconsumed)]] result& b) noexcept(meta::type<Err>::is_lvalue() ||
                                                                                                                         (INothrowSwappable<Err> && INothrowMoveConstructible<Err>))
        {
            using std::swap;
            switch (a.status)
            {
            case internal::result_status::ok:
                switch (b.status)
                {
                case internal::result_status::ok: break;
                case internal::result_status::error:
                    a.ctor_err(b.err(unsafe));
                    b.status = internal::result_status::ok;
                    break;
                default: swap(a.status, b.status);
                }
                break;
            case internal::result_status::error:
                switch (b.status)
                {
                case internal::result_status::ok:
                    a.ctor_err(b.err(unsafe));
                    b.status = internal::result_status::ok;
                    break;
                case internal::result_status::error:
                    swap(*a.storage.template data<internal::result_storage_type<Err>>(), *b.storage.template data<internal::result_storage_type<Err>>());
                    break;
                default: a.ctor_err(b.err(unsafe)); b.status = internal::result_status::empty;
                }
                break;
            default:
                switch (b.status)
                {
                case internal::result_status::ok: swap(a.status, b.status); break;
                case internal::result_status::error:
                    a.ctor_err(b.err(unsafe));
                    b.status = internal::result_status::empty;
                    break;
                default:;
                }
            }
        }

        friend struct sys::internal::result_b<result, void, Err>;
        friend struct sys::internal::result_b_err<result, void, Err>;
    };
} // namespace sys

namespace sys::internal
{
    /// @ingroup sys_internal
    /// @warning Be careful, `T` must be empty-queryable after moved-from, for an underlying specialization of `sys::result<...>` to be valid in its entirety!
    template <IResultStorable T>
    requires requires {
        requires !meta::type<T>::is_ref();
        meta::generic_nullable_adaptor<std::add_const_t<T>>(std::declval<std::add_const_t<T>&>()).is_null();
    }
    class [[nodiscard, clang::consumable(unconsumed)]] nullable_value_result : public internal::result_b<result, T, void>
    {
        T value {};
    public:
        template <typename... Args>
        requires (sizeof...(Args) > 0uz)
        constexpr /* NOLINT(hicpp-explicit-conversions) */ nullable_value_result(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...))) :
            value(std::forward<Args>(args)...)
        { }
        constexpr nullable_value_result(const nullable_value_result&) = delete;
        constexpr nullable_value_result([[clang::return_typestate(unknown)]] nullable_value_result&& other) noexcept(INothrowSwappable<T>) { swap(*this, other); }
        [[clang::callable_when("consumed", "unknown")]] constexpr ~nullable_value_result() = default;

        nullable_value_result& operator=(const nullable_value_result&) = delete;
        nullable_value_result& operator=([[clang::return_typestate(unknown)]] nullable_value_result&& other) noexcept(INothrowSwappable<T>)
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Whether the result is good.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(consumed)]] constexpr explicit operator bool() const noexcept
        {
            return !meta::generic_nullable_adaptor<std::add_const_t<T>>(this->value).is_null();
        }

        /// @brief Takes the value if the result has a good value.
        /// @pre `*this == true`
        [[nodiscard, clang::callable_when("consumed"), clang::set_typestate(unknown)]] constexpr T move() noexcept(INothrowMoveConstructible<T>)
        {
            _contract_assert(*this, "Taking value for a bad result!"); // LCOV_EXCL_LINE
            return std::move(this->value);
        }
        /// @brief `this->move()` if the result is good, otherwise `other`.
        template <typename With>
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr T move_or(With&& other) noexcept(
            INothrowMoveConstructible<T> && noexcept(T(std::forward<With>(other))))
        requires (!meta::type<T>::is_lvalue() || meta::type<With &&>::is_lvalue())
        {
            _retif(T(std::forward<With>(other)), !(*this));
            return std::move(this->value);
        }
        /// @brief Takes the value if the result has a good value.
        /// @pre `*this == true`
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr T expect() noexcept(INothrowMoveConstructible<T>)
        {
            _contract_assert(*this, "Taking value for a bad result!"); // LCOV_EXCL_LINE
            return std::move(this->value);
        }

        /// @brief Expects the result to be bad.
        /// @pre `*this == false`
        [[clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] void expect_err() const noexcept
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_LINE
        }

        /// @brief Apply `func(*this)` and return its output.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr auto transform(this result<T>&& _this, auto&& func) noexcept(
            noexcept(func(std::move(_this))))
        requires requires { func(std::move(_this)); }
        {
            return func(std::move(_this));
        }

        friend void swap(nullable_value_result& a, nullable_value_result& b) noexcept(INothrowSwappable<T>)
        {
            using std::swap;
            swap(a.value, b.value);
        }
    };
} // namespace sys::internal

namespace sys
{
    /// @ingroup sys
    template <typename T>
    class [[nodiscard]] result<T*, void> final : public internal::nullable_value_result<T*>
    {
    public:
        result() = delete;

        using internal::nullable_value_result<T*>::nullable_value_result;
        using internal::nullable_value_result<T*>::operator=;
    };
} // namespace sys
