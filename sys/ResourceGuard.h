#pragma once

#include <utility>

namespace sys
{
    /// @brief RAII guard for an acquired resource that needs to be released.
    /// @details Implements `std::is_nothrow_default_constructible_v`, `std::is_nothrow_move_constructible_v`, `std::is_nothrow_move_assignable_v`.
    /// @attention Lifetime assumptions!
    /// ```cpp
    /// void Release(T*) noexcept { /* ... */ }
    /// T res /* = ... */;
    /// ...
    /// sys::resource_guard<T, &Release> guard(res);
    /// ...
    /// guard.~resource_guard();
    /// ...
    /// res.~T();
    /// ```
    template <typename T, void (*Release)(T*) noexcept>
    struct resource_guard final
    {
    private:
        T* acq_res = nullptr;
    public:
        constexpr resource_guard() noexcept = default;
        /// @brief Guard `acq_res`.
        explicit resource_guard(T& acq_res) noexcept : acq_res(&acq_res) { }
        resource_guard(const resource_guard&) noexcept = delete;
        resource_guard(resource_guard&& other) noexcept { swap(*this, other); }
        ~resource_guard() noexcept
        {
            if (this->acq_res)
                Release(this->acq_res);
        }

        resource_guard& operator=(const resource_guard&) noexcept = delete;
        resource_guard& operator=(resource_guard&& other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Swap two `sys::resource_guard`.
        friend void swap(resource_guard& a, resource_guard& b) noexcept { std::swap(a.acq_res, b.acq_res); }
    };
}; // namespace sys
