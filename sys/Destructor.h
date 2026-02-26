#pragma once

#include <utility>

namespace sys
{
    /// @brief Wrapper for `noexcept` cleanup function.
    template <typename Func>
    requires (noexcept(std::declval<Func&>()()))
    struct destructor final
    {
        /// @brief Construct with a cleanup function.
        destructor(Func&& func) : func(std::move(func)) { } // NOLINT(hicpp-explicit-conversions)
        destructor(const destructor&) = delete;
        destructor(destructor&&) = delete;
        ~destructor() { func(); }

        destructor& operator=(const destructor&) = delete;
        destructor& operator=(destructor&&) = delete;
    private:
        Func func;
    };

    /// @brief Moveable, no-op-able, wrapper for `noexcept` cleanup function.
    template <typename Func>
    requires (noexcept(std::declval<Func&>()()))
    struct optional_destructor final
    {
        /// @brief Construct with a cleanup function.
        optional_destructor(Func&& func) : func(std::move(func)) { } // NOLINT(hicpp-explicit-conversions)
        optional_destructor(const optional_destructor&) = delete;
        /// @brief Moveable.
        optional_destructor(optional_destructor&& other) noexcept : func(std::move(other.func)), execute(other.execute) { other.execute = false; }
        ~optional_destructor()
        {
            if (this->execute) [[likely]]
                this->func();
        }

        optional_destructor& operator=(const optional_destructor&) = delete;
        /// @brief Move-assignable.
        optional_destructor& operator=(optional_destructor&& other) noexcept
        {
            if (this != &other) [[likely]]
            {
                this->func = std::move(other.func);
                this->execute = other.execute;
                other.execute = false;
            }
            return *this;
        }

        /// @brief Mark this `sys::destructor<...>` as no-op.
        void release() noexcept { this->execute = false; }
    private:
        Func func;
        bool execute = true;
    };
} // namespace sys
