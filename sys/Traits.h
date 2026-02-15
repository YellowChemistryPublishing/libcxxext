#pragma once

#include <concepts>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace sys::meta
{
    struct meta_type
    {
        meta_type() = delete;
    };

    template <bool Value>
    struct is_valid_prop : meta_type
    {
        static consteval bool is_valid() { return Value; }
    };

    template <bool Value>
    struct is_const_prop : meta_type
    {
        static consteval bool is_const() { return Value; }
    };
    template <bool Value>
    struct is_volatile_prop : meta_type
    {
        static consteval bool is_volatile() { return Value; }
    };
    template <bool Value>
    struct is_lvalue_ref_prop : meta_type
    {
        static consteval bool is_lvalue_ref() { return Value; }
    };
    template <bool Value>
    struct is_rvalue_ref_prop : meta_type
    {
        static consteval bool is_rvalue_ref() { return Value; }
    };
    template <bool Value>
    struct is_noexcept_prop : meta_type
    {
        static consteval bool is_noexcept() { return Value; }
    };

    template <bool Value>
    struct is_member_func_prop : meta_type
    {
        static consteval bool is_member_func() { return Value; }
    };

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

        template <typename>
        static consteval bool is_signature_of()
        {
            return false;
        }
    };

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

    template <typename... Pack>
    struct parameter_pack final : meta_type
    {
        using tuple = std::tuple<Pack...>;

        template <size_t Index>
        using at = std::tuple_element_t<Index, tuple>;

        template <typename T>
        static consteval bool contains()
        {
            return (std::same_as<T, Pack> || ...);
        }
    };

    template <bool Condition, typename T>
    struct type_case final : meta_type
    {
        using type = T;

        static consteval bool is_early_return() { return Condition; }
    };
    template <typename... Cases>
    struct type_switch_cases final : meta_type
    {
        using cases = std::tuple<Cases...>;
        using return_cases = decltype(std::tuple_cat(std::declval<std::conditional_t<Cases::is_early_return(), std::tuple<Cases>, std::tuple<>>>()...));

        template <size_t Index>
        using at = std::tuple_element_t<Index, cases>;

        static consteval size_t count_returns() { return (Cases::is_early_return() + ...); }
    };
    template <typename... Cases>
    using type_switch = std::tuple_element_t<0, typename type_switch_cases<Cases...>::return_cases>::type;

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
        else if constexpr (requires { requires false; })
        { }
    }
} // namespace sys::meta

namespace sys
{
    // NOLINTBEGIN(google-runtime-int)

    // @brief Built-in signed integer type.
    template <typename T>
    concept IBuiltinIntegerSigned =
        std::same_as<T, signed char> || std::same_as<T, signed short> || std::same_as<T, signed int> || std::same_as<T, signed long> || std::same_as<T, signed long long>;
    // @brief Built-in unsigned integer type.
    template <typename T>
    concept IBuiltinIntegerUnsigned =
        std::same_as<T, unsigned char> || std::same_as<T, unsigned short> || std::same_as<T, unsigned int> || std::same_as<T, unsigned long> || std::same_as<T, unsigned long long>;

    // NOLINTEND(google-runtime-int)

    // @brief Built-in integer type.
    template <typename T>
    concept IBuiltinInteger = IBuiltinIntegerSigned<T> || IBuiltinIntegerUnsigned<T>;
    // @brief Built-in floating-point type.
    template <typename T>
    concept IBuiltinFloatingPoint = std::same_as<T, float> || std::same_as<T, double> || std::same_as<T, long double>;
    // @brief Built-in numeric type.
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

    /// @brief Whether `T` can be pushed back into.
    template <typename T, typename... U>
    concept IAppendable = requires(T range) {
        requires (sizeof...(U) > 0);
        meta::append_to(range, std::declval<U>()...);
    } || requires {
        requires (sizeof...(U) == 0);
        requires (requires { &T::emplace_back; } || requires { &T::push_back; } || requires { &T::push; } || requires { &T::append; } || requires {
            std::declval<T>().emplace_back();
        } || requires { std::declval<T>().push_back(); } || requires { std::declval<T>().push(); } || requires { std::declval<T>().append(); });
    };
} // namespace sys
