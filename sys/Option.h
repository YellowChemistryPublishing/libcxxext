#pragma once

/// @file

#include <concepts>
#include <type_traits>

#include <Result.h>

namespace sys
{
    /// @ingroup sys
    /// @brief Represents an optional value. Every `sys::option` represents either `T` and contains a value, or `void` (by `return nullptr`) and does not.
    /// @note This is a convenience type alias for `result<T, void>`.
    /// @see For information on optional types, see [Rust Docs](https://doc.rust-lang.org/std/option/).
    template <typename T>
    requires (!std::same_as<std::remove_cvref_t<T>, void>)
    using option = result<T, void>;
} // namespace sys
