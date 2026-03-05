#pragma once

/// @file

#include <concepts>
#include <tuple>

#include <meta/Properties.h>

namespace sys::meta
{
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
} // namespace sys::meta

namespace sys
{
    /// @brief Whether `Functor` represents a function with signature `Signature`.
    template <typename Functor, typename Signature> // i.e. void(int, int)
    concept IFunc = meta::function_signature<Signature>::template is_signature_of<Functor>();
} // namespace sys
