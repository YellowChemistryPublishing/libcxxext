#pragma once

/// @file

#include <utility>

#include <LanguageSupport.h>

namespace sys
{
    /// @ingroup sys
    /// @brief RAII guard for an acquired resource that needs to be released.
    /// @details
    /// Implements `sys::INothrowDefaultConstructible`, `sys::INothrowMoveConstructible`, `sys::INothrowMoveAssignable`, `sys::INothrowDestructible`, `sys::INothrowSwappable`.
    /// @attention Lifetime assumptions!
    /// @code{.cpp}
    /// void Release(T*) noexcept { /* ... */ }
    /// T res /* = ... */;
    /// ...
    /// sys::resource_guard<T, &Release> guard(res);
    /// ...
    /// guard.~resource_guard();
    /// ...
    /// res.~T();
    /// @endcode
    template <typename T, void (*Release)(T*) noexcept>
    struct [[clang::scoped_lockable]] resource_guard final
    {
    private:
        T* resource = nullptr;
    public:
        constexpr resource_guard() noexcept = default;
        /// @brief Guard `resource`.
        /// @warning `unsafe` because `this` carries a "requires capability acquired" semantic.
        explicit resource_guard(T& resource, decltype(unsafe)) noexcept : resource(&resource) { }
        resource_guard(const resource_guard&) noexcept = delete;
        resource_guard(resource_guard&& other) noexcept { swap(*this, other); }
        ~resource_guard() noexcept
        {
            if (this->resource)
                Release(this->resource);
        }

        resource_guard& operator=(const resource_guard&) noexcept = delete;
        resource_guard& operator=(resource_guard&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        friend void swap(resource_guard& a, resource_guard& b) noexcept { std::swap(a.resource, b.resource); }
    };
}; // namespace sys
