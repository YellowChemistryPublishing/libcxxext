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
} // namespace sys::meta
