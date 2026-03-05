#pragma once

/// @file Destructor.h

#include <type_traits>
#include <utility>

#include <LanguageSupport.h>

namespace sys
{
    /// @brief Wrapper for `noexcept` cleanup function.
    /// @note Pass `byref`.
    template <typename Func>
    requires std::is_nothrow_invocable_v<Func&> && std::is_nothrow_move_constructible_v<Func> && std::is_nothrow_destructible_v<Func>
    struct destructor final
    {
        /// @brief Construct with a cleanup function.
        /* NOLINT(hicpp-explicit-conversions) */ destructor(Func&& func) noexcept : func(std::move(func)) { }
        destructor(const destructor&) = delete;
        destructor(destructor&&) = delete;
        ~destructor() { this->func(); }

        destructor& operator=(const destructor&) = delete;
        destructor& operator=(destructor&&) = delete;
    private:
        Func func;
    };

    template <typename Func>
    destructor(Func&&) -> destructor<std::remove_reference_t<Func>>;

    /// @brief Moveable, no-op-able, wrapper for `noexcept` cleanup function.
    /// @note Pass `byref`.
    template <typename Func>
    requires std::is_nothrow_invocable_v<Func&> && std::is_nothrow_move_constructible_v<Func> && std::is_nothrow_destructible_v<Func>
    struct optional_destructor final
    {
        /// @brief Construct with a cleanup function.
        /* NOLINT(hicpp-explicit-conversions) */ optional_destructor(Func&& func) noexcept : func(std::move(func)) { }
        optional_destructor(const optional_destructor&) = delete;
        optional_destructor(optional_destructor&& other) noexcept : func(std::move(other.func)), execute(other.execute)
        {
            if (other.execute)
            {
                other.func.~Func();
                other.execute = false;
            }
        }
        ~optional_destructor()
        {
            if (this->execute) [[likely]]
            {
                this->func();
                this->func.~Func();
            }
        }

        optional_destructor& operator=(const optional_destructor&) = delete;
        optional_destructor& operator=(optional_destructor&& other) noexcept
        {
            if (this != &other) [[likely]]
            {
                this->func = std::move(other.func);
                this->execute = other.execute;
                if (other.execute)
                {
                    other.func.~Func();
                    other.execute = false;
                }
            }
            return *this;
        }

        /// @brief Mark this `sys::destructor<...>` as no-op.
        /// @warning `unsafe` because you may only call this once.
        void release(unsafe) noexcept
        {
            this->func.~Func();
            this->execute = false;
        }
    private:
        union
        {
            Func func;
        };
        bool execute = true;
    };

    template <typename Func>
    optional_destructor(Func&&) -> optional_destructor<std::remove_reference_t<Func>>;
} // namespace sys
