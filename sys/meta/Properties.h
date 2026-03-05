#pragma once

// @file

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
} // namespace sys::meta
