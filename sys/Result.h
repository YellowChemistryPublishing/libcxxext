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
#include <meta/InterfaceRequirements.h>
#include <meta/NamedRequirements.h>
#include <meta/Nullable.h>
#include <meta/Type.h>
#include <meta/TypeSwitch.h>
#include <traits/RecurringTemplate.h>

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
    using result_storage_type = meta::type_switch<meta::type_case<std::same_as<std::remove_cvref_t<T>, void>, byte>,
                                                  meta::type_case<meta::type<T>::is_lvalue(), std::remove_reference_t<T>*>, meta::type_case<true, std::remove_cvref_t<T>>>;

    /// @internal
    /// @ingroup sys_internal
    /// @brief Shared functionality for result types.
    template <template <typename, typename> class Result, typename T, typename Err>
    struct result_b : public traits::recurring_template<Result<T, Err>>
    {
    protected:
        result_b() noexcept = default;
        result_b(const result_b&) noexcept = default;
        result_b(result_b&&) noexcept = default;
        ~result_b() noexcept = default;

        result_b& operator=(const result_b&) noexcept = default;
        result_b& operator=(result_b&&) noexcept = default;
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
    struct result_b_err : public traits::recurring_template<Result<T, Err>>
    {
    protected:
        result_b_err() noexcept = default;

        /// @internal
        /// @brief Constructs a result with an error.
        constexpr void ctor_err(auto&& val) noexcept(meta::type<Err>::is_lvalue() || noexcept(IConstructibleFrom<Err, decltype(val)>))
        requires requires {
            requires !meta::type<Err>::is_lvalue() || meta::type<decltype(val)>::is_lvalue();
            requires meta::type<Err>::is_lvalue() || IConstructibleFrom<Err, decltype(val)>;
        }
        {
            if constexpr (meta::type<Err>::is_lvalue())
                *this->downcast().storage.template data<result_storage_type<Err>>() = std::addressof(val);
            else
                std::construct_at(this->downcast().storage.template data<result_storage_type<Err>>(), _forward(val));
            this->downcast().status = result_status::error;
        }
        /// @internal
        /// @brief Inplace constructs a result with an error.
        constexpr void ctor_err(auto&&... args) noexcept(INothrowConstructibleFrom<Err, decltype(args)...>)
        requires IConstructibleFrom<Err, decltype(args)...>
        {
            std::construct_at(this->downcast().storage.template data<result_storage_type<Err>>(), _forward(args)...);
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
                return ret; // LCOV_EXCL_BR_LINE
            } // LCOV_EXCL_LINE
        }

        /// @internal
        /// @brief Convert to a result with a single error state.
        [[nodiscard]] constexpr explicit operator Result<T, void>() && noexcept(std::same_as<T, void> || meta::type<T>::is_lvalue() || requires {
            requires !std::same_as<T, void>;
            requires INothrowMoveConstructible<T>;
        })
        {
            switch (this->downcast().status) // LCOV_EXCL_BR_LINE
            {
            [[likely]] case result_status::ok:
                if constexpr (std::same_as<T, void>)
                    return Result<T, void>();
                else
                    return Result<T, void>(this->downcast().move(unsafe));
            [[unlikely]] case result_status::error:
                return Result<T, void>(nullptr);
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
    struct result_b_ok : public traits::recurring_template<Result<T, Err>>
    {
    protected:
        result_b_ok() noexcept = default;

        /// @internal
        /// @brief Constructs a result with a value.
        constexpr void ctor_ok(auto&& val) noexcept(meta::type<T>::is_lvalue() || INothrowConstructibleFrom<T, decltype(val)>)
        requires requires {
            requires !meta::type<T>::is_lvalue() || meta::type<decltype(val)>::is_lvalue();
            requires meta::type<T>::is_lvalue() || IConstructibleFrom<T, decltype(val)>;
        }
        {
            if constexpr (meta::type<T>::is_lvalue())
                *this->downcast().storage.template data<result_storage_type<T>>() = std::addressof(val);
            else
                std::construct_at(this->downcast().storage.template data<result_storage_type<T>>(), _forward(val));
            this->downcast().status = result_status::ok;
        }
        /// @internal
        /// @brief Inplace constructs a result with a value.
        constexpr void ctor_ok(auto&&... args) noexcept(INothrowConstructibleFrom<T, decltype(args)...>)
        requires requires {
            requires !meta::type<T>::is_lvalue();
            requires IConstructibleFrom<T, decltype(args)...>;
        }
        {
            std::construct_at(this->downcast().storage.template data<result_storage_type<T>>(), _forward(args)...);
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
                return ret; // LCOV_EXCL_BR_LINE
            } // LCOV_EXCL_LINE
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
            (meta::type<T>::is_unqualified() && !meta::type<T>::is_array() && (IMoveConstructible<T>) && (INothrowDestructible<T>));
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

        // To ensure well-defined conversion from any `Err` result to `void` result.
        constexpr explicit result(decltype(unsafe)) noexcept { };
    public:
        // NOLINTBEGIN(hicpp-explicit-conversions)

        /// @brief Construct a success result.
        [[clang::return_typestate(unconsumed)]] constexpr result() noexcept
        requires std::same_as<T, void>
            : status(internal::result_status::ok)
        { }
        /// @brief Constructs a result with a value.
        constexpr result(auto&& val) noexcept(meta::type<T>::is_lvalue() || INothrowConstructibleFrom<T, decltype(val)>)
        requires requires {
            requires !std::same_as<T, void>; // Result can hold a value and ...
            requires ((meta::type<T>::is_lvalue() && std::convertible_to<decltype(val), T>) ||
                      (std::same_as<_decltype_of(val), T> && IConstructibleFrom<T, decltype(val)>)); // `T` is reference or copy/move constructible.
        }
        {
            this->ctor_ok(_forward(val));
        }
        /// @brief Inplace constructs a result with a value.
        constexpr result(auto&&... args) noexcept(INothrowConstructibleFrom<T, decltype(args)...>)
        requires requires {
            requires !std::same_as<T, void>;                   // Result can hold a value and ...
            requires IConstructibleFrom<T, decltype(args)...>; // `T` is emplaceable from `args...`.
        }
        {
            this->ctor_ok(_forward(args)...);
        }

        /// @brief Construct an error result.
        constexpr result(std::nullptr_t) noexcept
        requires std::same_as<Err, void>
            : status(internal::result_status::error)
        { }
        /// @brief Inplace constructs a result with an error.
        constexpr result(decltype(error_tag), auto&&... args) noexcept(INothrowConstructibleFrom<Err, decltype(args)...>)
        requires requires {
            requires !std::same_as<Err, void>;                   // Result can hold an error and ...
            requires IConstructibleFrom<Err, decltype(args)...>; // `Err` is emplaceable from `args...`.
        }
        {
            this->ctor_err(_forward(args)...);
        }
        /// @brief Constructs a result with an error.
        /// @note Participates in overload resolution only if `T` and `Err` are distinct, or `std::same_as<T, void>`.
        constexpr result(auto&& err) noexcept(meta::type<Err>::is_lvalue() || INothrowConstructibleFrom<Err, decltype(err)>)
        requires requires {
            requires !std::same_as<Err, void>;                                                // Result can hold an error and ...
            requires !std::same_as<std::remove_reference_t<T>, std::remove_reference_t<Err>>; // Value and error types are distinct and ...
            requires ((meta::type<Err>::is_lvalue() && std::convertible_to<decltype(err), Err>) ||
                      (std::same_as<_decltype_of(err), Err> && IConstructibleFrom<Err, decltype(err)>)); // `Err` is reference or copy/move constructible.
        }
            : result(error_tag, _forward(err))
        { }
        /// @brief Inplace constructs a result with an error.
        /// @see `sys::result<T, Err>::result(auto&&...)`
        /// @note Participates in overload resolution only if `args...` cannot construct a `T`.
        constexpr result(auto&&... args) noexcept(INothrowConstructibleFrom<Err, decltype(args)...>)
        requires requires {
            requires !std::same_as<Err, void>;                   // Result can hold an error and ...
            requires !IConstructibleFrom<T, decltype(args)...>;  // Cannot construct `T` and ...
            requires IConstructibleFrom<Err, decltype(args)...>; // Can construct `Err`.
        }
            : result(error_tag, _forward(args)...)
        { }

        // NOLINTEND(hicpp-explicit-conversions)

        constexpr result(const result&) noexcept = delete;
        constexpr result(result&& other) noexcept((meta::type<T>::is_lvalue() || INothrowMoveConstructible<T> || std::same_as<T, void>) &&
                                                  (meta::type<Err>::is_lvalue() || INothrowMoveConstructible<Err> || std::same_as<Err, void>))
        {
            switch (other.status)
            {
            [[likely]] case internal::result_status::ok:
                if constexpr (!std::same_as<T, void>)
                    this->ctor_ok(other.move(unsafe));
                else
                {
                    this->status = internal::result_status::ok;
                    other.status = internal::result_status::empty;
                }
                break;
            [[unlikely]] case internal::result_status::error:
                if constexpr (!std::same_as<Err, void>)
                    this->ctor_err(other.err(unsafe));
                else
                {
                    this->status = internal::result_status::error;
                    other.status = internal::result_status::empty;
                }
                break;
            [[unlikely]] default:
                other.status = internal::result_status::empty;
            }
        }
        [[clang::callable_when("consumed", "unknown")]] constexpr ~result() noexcept
        {
            switch (this->status) // LCOV_EXCL_BR_LINE
            {
            [[likely]] case internal::result_status::ok:
                if constexpr (!meta::type<T>::is_lvalue() && !std::same_as<T, void>)
                    std::destroy_at(this->storage.template data<internal::result_storage_type<T>>());
                break;
            [[unlikely]] case internal::result_status::error:
                if constexpr (!meta::type<Err>::is_lvalue() && !std::same_as<Err, void>)
                    std::destroy_at(this->storage.template data<internal::result_storage_type<Err>>());
                break;
            [[likely]] default:;
            }
        }

        result& operator=(const result&) noexcept = delete;
        result& operator=(result&& other) noexcept((meta::type<T>::is_lvalue() || INothrowMoveConstructible<T> || std::same_as<T, void>) &&
                                                   (meta::type<Err>::is_lvalue() || INothrowMoveConstructible<Err> || std::same_as<Err, void>))
        {
            if (this == std::addressof(other)) [[unlikely]]
                return *this;

            switch (this->status)
            {
            [[likely]] case internal::result_status::ok:
                if constexpr (!meta::type<T>::is_lvalue() && !std::same_as<T, void>)
                    std::destroy_at(this->storage.template data<internal::result_storage_type<T>>());
                break;
            [[unlikely]] case internal::result_status::error:
                if constexpr (!meta::type<Err>::is_lvalue() && !std::same_as<Err, void>)
                    std::destroy_at(this->storage.template data<internal::result_storage_type<Err>>());
                break;
            [[unlikely]] default:;
            }
            this->status = internal::result_status::empty;
            switch (other.status)
            {
            [[likely]] case internal::result_status::ok:
                if constexpr (!std::same_as<T, void>)
                    this->ctor_ok(other.move(unsafe));
                else
                {
                    this->status = internal::result_status::ok;
                    other.status = internal::result_status::empty;
                }
                break;
            [[unlikely]] case internal::result_status::error:
                if constexpr (!std::same_as<Err, void>)
                    this->ctor_err(other.err(unsafe));
                else
                {
                    this->status = internal::result_status::error;
                    other.status = internal::result_status::empty;
                }
                break;
            [[unlikely]] default:
                other.status = internal::result_status::empty;
            }

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
        [[nodiscard, clang::set_typestate(unknown)]] constexpr explicit operator result<T, void>() && noexcept(meta::type<T>::is_lvalue() || INothrowMoveConstructible<T> ||
                                                                                                               std::same_as<T, void>)
        requires (!std::same_as<Err, void>)
        {
            _nowarn_begin_one_clang(_clwarn_clang_consumed);
            return _as(std::move(_as(*this, internal::result_b_err<result, T, Err>&)), result<T, void>);
            _nowarn_end_clang();
        }

        /// @brief Takes the value if the result has a good value.
        /// @pre `*this == true`
        [[nodiscard, clang::callable_when("consumed"), clang::set_typestate(unknown)]] constexpr T move()
        requires (!std::same_as<T, void>)
        {
            _contract_assert(this->status == internal::result_status::ok, "Taking value for a bad result!"); // LCOV_EXCL_BR_LINE
            return this->move(unsafe);
        }
        /// @brief `this->move()` if the result is good, otherwise `other`.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr T move_or(auto&&... inplace_other) noexcept(
            (meta::type<T>::is_lvalue() || (INothrowMoveConstructible<T> && INothrowConstructibleFrom<T, decltype(inplace_other)...>)))
        requires requires {
            requires !std::same_as<T, void>; // Result can hold a value and ...
            requires (requires {
                // `T` is non-reference and constructible from `inplace_other...`, otherwise ...
                requires !meta::type<T>::is_lvalue();
                requires IConstructibleFrom<T, decltype(inplace_other)...>;
            } || requires {
                // `T` is reference and `other` is not xvalue.
                requires sizeof...(inplace_other) == 1uz;
                requires meta::type<typename meta::parameter_pack<decltype(inplace_other)...>::template at<0>>::is_lvalue();
            });
        }
        {
            _retif(T(_forward(inplace_other)...), this->status != internal::result_status::ok);
            return this->move(unsafe);
        }
        /// @brief Takes the value if the result has a good value.
        /// @pre `*this == true`
        [[clang::set_typestate(consumed)]] constexpr T expect()
        {
            _contract_assert(this->status == internal::result_status::ok, "Taking value for a bad result!"); // LCOV_EXCL_BR_LINE
            if constexpr (!std::same_as<T, void>)
                return this->move(unsafe);
            else
                this->status = internal::result_status::empty;
        }

        /// @brief Take the error of a bad result.
        /// @pre `*this == false`
        [[nodiscard, clang::callable_when("consumed"), clang::set_typestate(unknown)]] constexpr Err err()
        requires (!std::same_as<Err, void> /* Result can hold an error and ... */ &&
                  (!meta::type<Err>::is_lvalue() /* `Err` is non-reference, otherwise ... */ || meta::type<Err>::is_lvalue() /* `Err` is reference and `other` is not xvalue. */))
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_BR_LINE
            return this->err(unsafe);
        }
        /// @brief Take the error of a bad result.
        /// @pre `*this == false`
        [[clang::set_typestate(consumed)]] constexpr Err expect_err()
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_BR_LINE
            if constexpr (!std::same_as<Err, void>)
                return this->err(unsafe);
            else
                this->status = internal::result_status::empty;
        }

        /// @brief Apply `func(*this)` and return its output.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr auto transform(ICallable<result&&> auto&& func) && noexcept(
            INothrowCallable<decltype(func), result&&>)
        {
            return func(std::move(*this));
        }

        friend void swap(result& a,
                         result& b) noexcept((meta::type<T>::is_lvalue() || INothrowMoveConstructible<T>) && (meta::type<Err>::is_lvalue() || INothrowMoveConstructible<Err>))

        {
            if ((b.status == internal::result_status::ok && sizeof(internal::result_storage_type<T>) < sizeof(internal::result_storage_type<Err>)) ||
                (b.status == internal::result_status::error && sizeof(internal::result_storage_type<Err>) <= sizeof(internal::result_storage_type<T>)) ||
                b.status == internal::result_status::empty)
            {
                result tmp = std::move(b);
                b = std::move(a);
                a = std::move(tmp);
            }
            else
            {
                result tmp = std::move(a);
                a = std::move(b);
                b = std::move(tmp);
            }
        }

        friend struct sys::internal::result_b<result, T, Err>;
        friend struct sys::internal::result_b_ok<result, T, Err>;
        template <template <typename, typename> class, typename, typename>
        friend struct sys::internal::result_b_err;
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
        constexpr result(result&& other) noexcept : status(other.status) { other.status = internal::result_status::empty; }
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
        [[clang::set_typestate(consumed)]] void expect()
        {
            _contract_assert(this->status == internal::result_status::ok, "Taking value for a bad or empty result!"); // LCOV_EXCL_BR_LINE
            this->status = internal::result_status::empty;
        }
        /// @brief Expects the result to be bad.
        /// @pre `*this == false`
        [[clang::set_typestate(consumed)]] void expect_err()
        {
            _contract_assert(this->status == internal::result_status::error, "Taking error for a good or empty result!"); // LCOV_EXCL_BR_LINE
            this->status = internal::result_status::empty;
        }

        /// @brief Apply `func(*this)` and return its output.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr auto transform(ICallable<result&&> auto&& func) && noexcept(
            noexcept(func(std::move(*this))))
        {
            return func(std::move(*this));
        }

        friend void swap(result& a, result& b) noexcept { std::swap(a.status, b.status); }

        friend struct sys::internal::result_b<result, void, void>;
        template <template <typename, typename> class, typename, typename>
        friend struct sys::internal::result_b_err;
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
        constexpr /* NOLINT(hicpp-explicit-conversions) */ nullable_value_result(auto&&... args) noexcept(INothrowConstructibleFrom<T, decltype(args)...>)
        requires requires {
            requires sizeof...(args) > 0uz;
            requires IConstructibleFrom<T, decltype(args)...>;
        }
            : value(_forward(args)...)
        { }
        constexpr nullable_value_result(const nullable_value_result&) = delete;
        constexpr nullable_value_result(nullable_value_result&& other) noexcept(INothrowSwappable<T>) { swap(*this, other); }
        [[clang::callable_when("consumed", "unknown")]] constexpr ~nullable_value_result() = default;

        nullable_value_result& operator=(const nullable_value_result&) = delete;
        nullable_value_result& operator=(nullable_value_result&& other) noexcept(INothrowSwappable<T>)
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
        [[nodiscard, clang::callable_when("consumed"), clang::set_typestate(unknown)]] constexpr T move()
        {
            _contract_assert(*this, "Taking value for a bad result!"); // LCOV_EXCL_BR_LINE
            return std::move(this->value);
        }
        /// @brief `this->move()` if the result is good, otherwise `other`.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr T move_or(auto&& other) noexcept(
            INothrowMoveConstructible<T> && INothrowConstructibleFrom<T, decltype(other)>)
        requires requires {
            requires !meta::type<T>::is_lvalue() || meta::type<decltype(other)>::is_lvalue();
            requires IConstructibleFrom<T, decltype(other)>;
        }
        {
            _retif(T(_forward(other)), !*this);
            return std::move(this->value);
        }
        /// @brief Takes the value if the result has a good value.
        /// @pre `*this == true`
        [[clang::set_typestate(consumed)]] constexpr T expect()
        {
            _contract_assert(*this, "Taking value for a bad result!"); // LCOV_EXCL_BR_LINE
            return std::move(this->value);
        }

        /// @brief Expects the result to be bad.
        /// @pre `*this == false`
        [[clang::set_typestate(consumed)]] void expect_err() const
        {
            _contract_assert(!*this, "Taking error for a good or empty result!"); // LCOV_EXCL_BR_LINE
        }

        /// @brief Apply `func(*this)` and return its output.
        [[nodiscard, clang::callable_when("consumed", "unconsumed"), clang::set_typestate(unknown)]] constexpr auto transform(
            this result<T>&& _this, ICallable<result<T>&&> auto&& func) noexcept(noexcept(func(std::move(_this))))
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
    requires (meta::type<T>::is_unqualified() && IObject<T>)
    class [[nodiscard]] result<T*, void> final : public internal::nullable_value_result<T*>
    {
    public:
        result() = delete;

        using internal::nullable_value_result<T*>::nullable_value_result;
        using internal::nullable_value_result<T*>::operator=;
    };
} // namespace sys

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
