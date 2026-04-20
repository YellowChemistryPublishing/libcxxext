#pragma once

/// @file

#include <type_traits>

namespace sys
{
    /// @ingroup sys
    /// @brief Interface for functor types that can be nothrow invoked.
    template <typename Func, typename... Args>
    concept INothrowInvocable = std::is_nothrow_invocable_v<Func, Args...>;
} // namespace sys
