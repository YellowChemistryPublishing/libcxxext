#pragma once

namespace sys::traits
{
    /// @ingroup sys
    /// @brief As labelled on box--an empty struct, occasionally useful for conditional inheritance.
    /// @details Implements `sys::ICStructEquivalent`, `sys::INothrowDefaultConstructible`, `sys::ITriviallyCopyable`, `sys::ITriviallyMoveable`, `sys::INothrowDestructible`.
    struct empty
    { };
} // namespace sys::traits
