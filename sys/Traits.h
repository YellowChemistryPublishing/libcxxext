#pragma once

/// @file Traits.h

#include <concepts>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <LanguageSupport.h>

namespace sys::meta
{
    /// @internal
    /// @brief Shared attributes for all meta types.
    /// @note Static class.
    struct meta_type
    {
        meta_type() = delete;
    };

    /// @internal
    /// @brief Metadata validity property.
    /// @note Static class.
    template <bool Value>
    struct is_valid_prop : meta_type
    {
        static consteval bool is_valid() { return Value; }
    };
    /// @internal
    /// @brief Metadata has `const` attribute.
    /// @note Static class.
    template <bool Value>
    struct is_const_prop : meta_type
    {
        static consteval bool is_const() { return Value; }
    };
    /// @internal
    /// @brief Metadata has `volatile` attribute.
    /// @note Static class.
    template <bool Value>
    struct is_volatile_prop : meta_type
    {
        static consteval bool is_volatile() { return Value; }
    };
    /// @internal
    /// @brief Metadata is lvalue reference attribute.
    /// @note Static class.
    template <bool Value>
    struct is_lvalue_ref_prop : meta_type
    {
        static consteval bool is_lvalue_ref() { return Value; }
    };
    /// @internal
    /// @brief Metadata is rvalue reference attribute.
    /// @note Static class.
    template <bool Value>
    struct is_rvalue_ref_prop : meta_type
    {
        static consteval bool is_rvalue_ref() { return Value; }
    };
    /// @internal
    /// @brief Metadata has `noexcept` attribute.
    /// @note Static class.
    template <bool Value>
    struct is_noexcept_prop : meta_type
    {
        static consteval bool is_noexcept() { return Value; }
    };
    /// @internal
    /// @brief Metadata is member function attribute.
    /// @note Static class.
    template <bool Value>
    struct is_member_func_prop : meta_type
    {
        static consteval bool is_member_func() { return Value; }
    };

    /// @brief Metadata for some function signature `T`.
    /// @note Static class.
    template <typename T = void, typename... Args>
    struct function_signature : is_valid_prop<false>,
                                is_const_prop<false>,
                                is_volatile_prop<false>,
                                is_lvalue_ref_prop<false>,
                                is_rvalue_ref_prop<false>,
                                is_noexcept_prop<false>,
                                is_member_func_prop<false>
    {
        using return_type = T;
        using arguments = std::tuple<Args...>;

        using is_valid_prop<false>::is_valid;
        using is_const_prop<false>::is_const;
        using is_volatile_prop<false>::is_volatile;
        using is_lvalue_ref_prop<false>::is_lvalue_ref;
        using is_rvalue_ref_prop<false>::is_rvalue_ref;
        using is_noexcept_prop<false>::is_noexcept;
        using is_member_func_prop<false>::is_member_func;

        /// @brief Whether typename `T` is of some signature.
        template <typename>
        static consteval bool is_signature_of()
        {
            return false;
        }
    };

    /// @cond
    template <typename T, typename... Args>
    struct function_signature<T(Args...)> : function_signature<T, Args...>, is_valid_prop<true>
    {
        using is_valid_prop<true>::is_valid;

        template <typename Func>
        static consteval bool is_signature_of()
        {
            return requires {
                { std::declval<Func>()(std::declval<Args>()...) } -> std::same_as<T>;
            };
        }
    };
    template <typename T, typename... Args>
    struct function_signature<T (&)(Args...)> : function_signature<T(Args...)>
    { };
    template <typename T, typename... Args>
    struct function_signature<T (*)(Args...)> : function_signature<T(Args...)>
    { };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...)> : function_signature<T, Args...>, is_valid_prop<true>, is_member_func_prop<true>
    {
        using is_valid_prop<true>::is_valid;
        using is_member_func_prop<true>::is_member_func;

        template <typename Func>
        static consteval bool is_signature_of()
        {
            return requires {
                { std::declval<Func>()(std::declval<For*>(), std::declval<Args>()...) } -> std::same_as<T>;
            };
        }
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const> : function_signature<T (For::*)(Args...)>, is_const_prop<true>
    {
        using is_const_prop<true>::is_const;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) volatile> : function_signature<T (For::*)(Args...)>, is_volatile_prop<true>
    {
        using is_volatile_prop<true>::is_volatile;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const volatile> : function_signature<T (For::*)(Args...) const>, is_volatile_prop<true>
    {
        using is_volatile_prop<true>::is_volatile;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) noexcept> : function_signature<T (For::*)(Args...)>, is_noexcept_prop<true>
    {
        using is_noexcept_prop<true>::is_noexcept;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const noexcept> : function_signature<T (For::*)(Args...) const>, is_noexcept_prop<true>
    {
        using is_noexcept_prop<true>::is_noexcept;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) volatile noexcept> : function_signature<T (For::*)(Args...) volatile>, is_noexcept_prop<true>
    {
        using is_noexcept_prop<true>::is_noexcept;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const volatile noexcept> : function_signature<T (For::*)(Args...) const volatile>, is_noexcept_prop<true>
    {
        using is_noexcept_prop<true>::is_noexcept;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) /* Appease `clang-format`. */&> : function_signature<T (For::*)(Args...)>, is_lvalue_ref_prop<true>
    {
        using is_lvalue_ref_prop<true>::is_lvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) &&> : function_signature<T (For::*)(Args...)>, is_rvalue_ref_prop<true>
    {
        using is_rvalue_ref_prop<true>::is_rvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const&> : function_signature<T (For::*)(Args...) const>, is_lvalue_ref_prop<true>
    {
        using is_lvalue_ref_prop<true>::is_lvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const&&> : function_signature<T (For::*)(Args...) const>, is_rvalue_ref_prop<true>
    {
        using is_rvalue_ref_prop<true>::is_rvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) volatile&> : function_signature<T (For::*)(Args...) volatile>, is_lvalue_ref_prop<true>
    {
        using is_lvalue_ref_prop<true>::is_lvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) volatile&&> : function_signature<T (For::*)(Args...) volatile>, is_rvalue_ref_prop<true>
    {
        using is_rvalue_ref_prop<true>::is_rvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const volatile&> : function_signature<T (For::*)(Args...) const volatile>, is_lvalue_ref_prop<true>
    {
        using is_lvalue_ref_prop<true>::is_lvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const volatile&&> : function_signature<T (For::*)(Args...) const volatile>, is_rvalue_ref_prop<true>
    {
        using is_rvalue_ref_prop<true>::is_rvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) & noexcept> : function_signature<T (For::*)(Args...) noexcept>, is_lvalue_ref_prop<true>
    {
        using is_lvalue_ref_prop<true>::is_lvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) && noexcept> : function_signature<T (For::*)(Args...) noexcept>, is_rvalue_ref_prop<true>
    {
        using is_rvalue_ref_prop<true>::is_rvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const & noexcept> : function_signature<T (For::*)(Args...) const noexcept>, is_lvalue_ref_prop<true>
    {
        using is_lvalue_ref_prop<true>::is_lvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const && noexcept> : function_signature<T (For::*)(Args...) const noexcept>, is_rvalue_ref_prop<true>
    {
        using is_rvalue_ref_prop<true>::is_rvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) volatile & noexcept> : function_signature<T (For::*)(Args...) volatile noexcept>, is_lvalue_ref_prop<true>
    {
        using is_lvalue_ref_prop<true>::is_lvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) volatile && noexcept> : function_signature<T (For::*)(Args...) volatile noexcept>, is_rvalue_ref_prop<true>
    {
        using is_rvalue_ref_prop<true>::is_rvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const volatile & noexcept> : function_signature<T (For::*)(Args...) const volatile noexcept>, is_lvalue_ref_prop<true>
    {
        using is_lvalue_ref_prop<true>::is_lvalue_ref;
    };
    template <typename For, typename T, typename... Args>
    struct function_signature<T (For::*)(Args...) const volatile && noexcept> : function_signature<T (For::*)(Args...) const volatile noexcept>, is_rvalue_ref_prop<true>
    {
        using is_rvalue_ref_prop<true>::is_rvalue_ref;
    };
    /// @endcond

    /// @brief Metadata for parameter pack `Pack...`.
    /// @note Static class.
    template <typename... Pack>
    struct parameter_pack final : meta_type
    {
        using tuple = std::tuple<Pack...>;

        template <size_t Index>
        using at = std::tuple_element_t<Index, tuple>;

        /// @brief Whether parameter pack contains `T`.
        template <typename T>
        static consteval bool contains()
        {
            return (std::same_as<T, Pack> || ...);
        }
    };

    /// @brief Metadata for a template type.
    /// @note Static class.
    template <typename T>
    struct template_type final : meta_type
    {
    private:
        template <typename, template <typename...> class>
        struct is_templated_from final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <typename...> class Template, typename... Args>
        struct is_templated_from<Template<Args...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
    public:
        /// @brief Whether `T` is defined from `Template`.
        template <template <typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from<T, Template>::value();
        }
    };

    /// @brief Obtain the type `T`, inherited with the `cv` qualifiers of a reference-stripped `With`.
    template <typename T, typename With>
    using replace_cv = std::conditional_t<
        std::is_const_v<std::remove_reference_t<With>>,
        std::add_const_t<std::conditional_t<std::is_volatile_v<std::remove_reference_t<With>>, std::add_volatile_t<std::remove_cvref_t<T>>, std::remove_cvref_t<T>>>,
        std::conditional_t<std::is_volatile_v<std::remove_reference_t<With>>, std::add_volatile_t<std::remove_cvref_t<T>>, std::remove_cvref_t<T>>>;

    /// @brief Metadata type for `sys::meta::type_switch_cases<...>`.
    template <bool Condition, typename T>
    struct type_case final : meta_type
    {
        using type = T;

        /// @brief Whether `Condition` is `true`.
        static consteval bool is_early_return() { return Condition; }
    };
    /// @brief Metadata type for a type-`switch`.
    template <typename... Cases>
    struct type_switch_cases final : meta_type
    {
        using cases = std::tuple<Cases...>;
        using return_cases = decltype(std::tuple_cat(std::declval<std::conditional_t<Cases::is_early_return(), std::tuple<Cases>, std::tuple<>>>()...));

        template <size_t Index>
        using at = std::tuple_element_t<Index, cases>;

        /// @brief How many cases meet their conditions.
        static consteval size_t count_returns() { return (Cases::is_early_return() + ...); }
    };
    /// @brief The type of the first type-case that meets its condition.
    template <typename... Cases>
    using type_switch = std::tuple_element_t<0, typename type_switch_cases<Cases...>::return_cases>::type;

    /// @brief Check whether an empty-queryable `range` is empty.
    template <typename T>
    constexpr bool is_empty(const T& range)
    {
        if constexpr (requires { range.empty(); })
            return range.empty();
        else if constexpr (requires { range.size(); })
            return std::size(range) == _as(decltype(std::size(range)), 0);
        else if constexpr (requires { requires false; })
        { }
    }
    /// @brief Inplace construct and append to an appendable `range`.
    template <typename T, typename... Args>
    constexpr decltype(auto) append_to(T& range, Args&&... args)
    {
        if constexpr (requires { range.emplace_back(std::forward<Args>(args)...); })
            return range.emplace_back(std::forward<Args>(args)...);
        else if constexpr (requires { range.push_back(std::forward<Args>(args)...); })
            return range.push_back(std::forward<Args>(args)...);
        else if constexpr (requires { range.push(std::forward<Args>(args)...); })
            return range.push(std::forward<Args>(args)...);
        else if constexpr (requires { range.append(std::forward<Args>(args)...); })
            return range.append(std::forward<Args>(args)...);
        else if constexpr (requires { (range << ... << std::forward<Args>(args)); })
            return (range << ... << std::forward<Args>(args));
        else if constexpr (requires { requires false; })
        { }
    }
} // namespace sys::meta

namespace sys
{
    // NOLINTBEGIN(google-runtime-int)

    /// @brief Built-in signed integer type.
    template <typename T>
    concept IBuiltinIntegerSigned =
        std::same_as<T, signed char> || std::same_as<T, signed short> || std::same_as<T, signed int> || std::same_as<T, signed long> || std::same_as<T, signed long long>;
    /// @brief Built-in unsigned integer type.
    template <typename T>
    concept IBuiltinIntegerUnsigned =
        std::same_as<T, unsigned char> || std::same_as<T, unsigned short> || std::same_as<T, unsigned int> || std::same_as<T, unsigned long> || std::same_as<T, unsigned long long>;

    // NOLINTEND(google-runtime-int)

    /// @brief Built-in integer type.
    template <typename T>
    concept IBuiltinInteger = IBuiltinIntegerSigned<T> || IBuiltinIntegerUnsigned<T>;
    /// @brief Built-in floating-point type.
    template <typename T>
    concept IBuiltinFloatingPoint = std::same_as<T, float> || std::same_as<T, double> || std::same_as<T, long double>;
    /// @brief Built-in numeric type.
    template <typename T>
    concept IBuiltinNumeric = IBuiltinInteger<T> || IBuiltinFloatingPoint<T>;

    /// @brief Whether `T` can hold the entire range of `CanHold`.
    template <typename T, typename CanHold>
    concept IBuiltinIntegerCanHold =
        IBuiltinInteger<T> && IBuiltinInteger<CanHold> && std::cmp_less_equal(std::numeric_limits<T>::lowest(), std::numeric_limits<CanHold>::lowest()) &&
        std::cmp_greater_equal(std::numeric_limits<T>::max(), std::numeric_limits<CanHold>::max());

    /// @brief Whether `T` represents a unicode character.
    template <typename T>
    concept IUnicodeCharacter = std::same_as<T, char8_t> || std::same_as<T, char16_t> || std::same_as<T, char32_t>;
    /// @brief Whether `T` is a character type.
    template <typename T>
    concept ICharacter = std::same_as<T, char> || std::same_as<T, wchar_t> || IUnicodeCharacter<T>;

    /// @brief Whether `Functor` represents a function with signature `Signature`.
    template <typename Functor, typename Signature> // i.e. void(int, int)
    concept IFunc = meta::function_signature<Signature>::template is_signature_of<Functor>();

    /// @brief Whether `T` is sizeable.
    template <typename T, typename U = size_t>
    concept ISizeable = requires(T range) {
        requires !std::same_as<U, void>;
        { std::size(range) } -> std::same_as<U>;
    } || requires(T range) {
        requires std::same_as<U, void>;
        std::size(range);
    };

    /// @brief Whether `T` is iterable.
    template <typename T, typename U = void>
    concept IEnumerable = requires(T& range, std::remove_cvref_t<decltype(std::begin(range))> it) {
        std::begin(range);
        std::end(range);

        std::begin(range) != std::end(range);
        ++it;

        requires (requires {
            requires std::same_as<U, void>;
            *std::begin(range);
        } || std::same_as<std::remove_cvref_t<decltype(*std::begin(range))>, U>);
    };

    /// @brief Whether `T` can be checked for emptiness.
    template <typename T>
    concept IEmptyQueryable = requires(T range) { meta::is_empty(range); };
    /// @brief Whether `T` can be appended to.
    template <typename T, typename... U>
    concept IAppendable = requires(T range) {
        requires (sizeof...(U) > 0);
        meta::append_to(range, std::declval<U>()...);
    };
} // namespace sys
