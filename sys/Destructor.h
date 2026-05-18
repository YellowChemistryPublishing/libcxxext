#pragma once

/// @file

#include <type_traits>
#include <utility>

#include <LanguageSupport.h>
#include <meta/InterfaceRequirements.h>
#include <meta/NamedRequirements.h>

namespace sys
{
    /// @ingroup sys
    /// @brief Wrapper for `noexcept` cleanup function.
    /// @note Pass `byref`.
    /// @details Implements `sys::INothrowDestructible`.
    template <typename Func>
    requires INothrowCallable<Func&> && INothrowDestructible<Func>
    struct [[clang::scoped_lockable]] destructor final
    {
        /// @brief Construct with a cleanup function.
        /* NOLINT(hicpp-explicit-conversions) */ destructor(Func&& func) noexcept(INothrowMoveConstructible<Func>) : func(std::move(func)) { }
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

    /// @ingroup sys
    /// @brief Moveable, no-op-able, wrapper for `noexcept` cleanup function.
    /// @note Pass `byref`.
    /// @details
    /// Implements `sys::IMoveConstructible`, `sys::IMoveAssignable`, `sys::INothrowDestructible`.
    /// Conditionally implements `sys::INothrowMoveConstructible`, `sys::INothrowMoveAssignable`.
    template <typename Func>
    requires INothrowCallable<Func&> && INothrowDestructible<Func>
    struct [[clang::scoped_lockable]] optional_destructor final
    {
        /// @brief Construct with a cleanup function.
        /* NOLINT(hicpp-explicit-conversions) */ optional_destructor(Func&& func) noexcept(INothrowMoveConstructible<Func>) : func(std::move(func)) { }
        optional_destructor(const optional_destructor&) = delete;
        optional_destructor(optional_destructor&& other) noexcept(INothrowMoveConstructible<Func>) : func(std::move(other.func)), execute(other.execute) { other.execute = false; }
        ~optional_destructor()
        {
            if (this->execute)
                this->func();
        }

        optional_destructor& operator=(const optional_destructor&) = delete;
        optional_destructor& operator=(optional_destructor&& other) noexcept(INothrowMoveAssignable<Func>)
        {
            _retif(*this, this == &other);

            this->func = std::move(other.func);
            this->execute = other.execute;
            other.execute = false;
            return *this;
        }

        /// @brief Mark this `sys::destructor<...>` as no-op.
        void clear() noexcept { this->execute = false; }
    private:
        Func func;
        bool execute = true;
    };

    template <typename Func>
    optional_destructor(Func&&) -> optional_destructor<std::remove_reference_t<Func>>;
} // namespace sys

/// @def _defer(...)
/// @ingroup sys
/// @brief Defer a nothrow callable to execute at the end of the current scope, with a `sys::destructor<...>`.
#define _defer(...) const ::sys::destructor _ppcat(_deferred, __LINE__) = (__VA_ARGS__);
