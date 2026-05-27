#pragma once

/// @file

namespace sys::meta
{
    /// @ingroup sys
    /// @brief Shared attributes for all meta types.
    struct /* [[sys::static]] */ meta_type
    {
        meta_type() = delete;
    };

    /// @ingroup sys
    /// @brief Metadata validity property.
    template <bool Value>
    struct /* [[sys::static]] */ is_valid_prop : meta_type
    {
        static consteval bool is_valid() { return Value; }
    };

    /// @ingroup sys
    /// @brief Metadata is unqualified and non-reference attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_unqualified_prop : meta_type
    {
        static consteval bool is_unqualified() { return Value; }
    };

    /// @ingroup sys
    /// @brief Metadata has `const` attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_const_prop : meta_type
    {
        static consteval bool is_const() { return Value; }
    };
    /// @ingroup sys
    /// @brief Metadata has `volatile` attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_volatile_prop : meta_type
    {
        static consteval bool is_volatile() { return Value; }
    };
    /// @ingroup sys
    /// @brief Metadata has `const` _or_ `volatile` attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_cv_qualified_prop : meta_type
    {
        static consteval bool is_cv_qualified() { return Value; }
    };

    /// @ingroup sys
    /// @brief Metadata is lvalue reference attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_lvalue_prop : meta_type
    {
        static consteval bool is_lvalue_ref() { return Value; }
    };
    /// @ingroup sys
    /// @brief Metadata is rvalue reference attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_rvalue_prop : meta_type
    {
        static consteval bool is_rvalue_ref() { return Value; }
    };
    /// @ingroup sys
    /// @brief Metadata is reference attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_ref_prop : meta_type
    {
        static consteval bool is_ref() { return Value; }
    };

    /// @ingroup sys
    /// @brief Metadata is C-array attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_array_prop : meta_type
    {
        static consteval bool is_array() { return Value; }
    };

    /// @ingroup sys
    /// @brief Metadata has `noexcept` attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_noexcept_prop : meta_type
    {
        static consteval bool is_noexcept() { return Value; }
    };
    /// @ingroup sys
    /// @brief Metadata is member function attribute.
    template <bool Value>
    struct /* [[sys::static]] */ is_member_func_prop : meta_type
    {
        static consteval bool is_member_func() { return Value; }
    };
} // namespace sys::meta
