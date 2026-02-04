#pragma once

#include <Platform.h>

#include <cstdio> // NOLINT(misc-include-cleaner)
#if _libcxxext_compiler_gcc
#include <cxxabi.h>
#endif
#include <exception>
#include <memory>
#include <print>
#include <utility>

namespace sys
{
    /// @brief Base type for all managed exceptions.
    /// @note
    /// Inherits from `std::exception`, no need to catch this type specifically.
    /// Pass `byref`.
    struct managed_exception : public std::exception
    {
        /// @brief What went wrong.
        [[nodiscard]] constexpr const char* what() const noexcept override = 0;
    };

#if _libcxxext_compiler_gcc
    inline std::unique_ptr<char, void (*)(void*)> exception_type_name(std::exception_ptr ex)
    {
        return { abi::__cxa_demangle(ex.__cxa_exception_type()->name() /* Contract implied: `name != nullptr`. */, NULL, NULL, NULL), std::free };
    }
#else
    /// @brief If platform support exists, obtain a human-readable name of the exception type.
    inline std::unique_ptr<const char, void (*)(const void*)> exception_type_name([[maybe_unused]] std::exception_ptr ex)
    {
        return { "<unknown>", [](const void* const) { } };
    }
#endif

    /// @brief Terminate the program.
    /// @note Pass `byref`.
    struct terminate_exception : public managed_exception
    {
        /// @brief On construction, terminate the program.
        terminate_exception() noexcept { std::terminate(); }

        /// @brief Never called.
        /// @return `[[noreturn]]`
        [[noreturn]] const char* what() const noexcept override { std::unreachable(); }
    };
    /// @brief Thrown when a contract is violated.
    /// @note
    /// Prefer using the `_contract_assert` macro instead. Don't catch this exception.
    /// Pass `byref`.
    struct contract_violation_exception : public managed_exception
    {
        /// @brief Construct the exception with a reason, which is logged on construction.
        /// @param why The reason the contract was violated.
        /// @attention Lifetime assumptions!
        /// ```cpp
        /// static const char* reason = /* Literal string. */;
        /// ...
        /// _throw(sys::contract_violation_exception(reason));
        /// ```
        contract_violation_exception(const char* why) : why(why) // NOLINT(hicpp-explicit-conversions)
        {
            std::println(stderr, "Note: {}", why);
        }

        /// @brief Obtain the reason a contract was violated.
        [[nodiscard]] const char* what() const noexcept override { return this->why; }
    private:
        const char* why;
    };
} // namespace sys
