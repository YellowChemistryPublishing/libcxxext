#pragma once

#include <utility>

namespace sys
{
    template <typename Func>
    requires (noexcept(std::declval<Func&>()()))
    struct destructor
    {
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        destructor(Func&& func) : func(std::move(func)) { }
        destructor(const destructor&) = delete;
        destructor(destructor&&) = delete;
        ~destructor() { func(); }

        destructor& operator=(const destructor&) = delete;
        destructor& operator=(destructor&&) = delete;
    private:
        Func func;
    };

    template <typename Func>
    requires (noexcept(std::declval<Func&>()()))
    struct optional_destructor
    {
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        optional_destructor(Func&& func) : func(std::move(func)) { }
        optional_destructor(const optional_destructor&) = delete;
        optional_destructor(optional_destructor&& other) noexcept : func(std::move(other.func)), execute(other.execute) { other.execute = false; }
        ~optional_destructor()
        {
            if (this->execute) [[likely]]
                this->func();
        }

        optional_destructor& operator=(const optional_destructor&) = delete;
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

        void release() noexcept { this->execute = false; }
    private:
        Func func;
        bool execute = true;
    };
} // namespace sys
