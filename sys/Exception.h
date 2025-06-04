#pragma once

#include <cxxabi.h>
#include <memory>
#include <print>
#include <utility>

#include <CompilerWarnings.h>
#include <LanguageSupport.h>

namespace sys
{
    /// @brief Base type for all managed exceptions.
    /// @note
    /// Inherits from `std::exception`, no need to catch this type specifically.
    /// Pass `byref`.
    struct managed_exception : public std::exception
    {
        /// @brief What went wrong.
        /// @return The reason for the exception.
        constexpr virtual const char* what() const noexcept override = 0;
    };
    /// @brief Thrown when an `fenced_pointer<T>` is dereferenced, addition-assigned, subtraction-assigned, incremented, or decremented when `nullptr`.
    /// @note Pass `byref`.
    struct null_pointer_exception : public managed_exception
    {
        /// @brief What went wrong.
        /// @return Literal string.
        constexpr const char* what() const noexcept override
        {
            return "Attempted to access, increment, or decrement a null pointer.";
        }
    };

    /// @brief A pointer that throws on attempted `nullptr` dereference.
    /// @tparam T The type of the pointer.
    /// @note Pass `byval`.
    template <typename T>
    struct fenced_pointer
    {
        /// @brief Construct `fenced_pointer<T>` with the value `nullptr`.
        inline fenced_pointer() noexcept = default;
        /// @brief Construct `fenced_pointer<T>` with a value.
        /// @param value The value to assign to the pointer.
        inline fenced_pointer(T* value) noexcept : value(value)
        { }
        /// @brief Copy constructor.
        /// @param other The other `fenced_pointer<T>` to copy.
        inline fenced_pointer(const fenced_pointer& other) noexcept : value(other.value)
        { }

        /// @brief Assignment operator.
        /// @param value The value to assign to the pointer.
        /// @return Reference to `*this`.
        inline fenced_pointer& operator=(T* value) noexcept
        {
            this->value = value;
            return *this;
        }
        /// @brief Copy assignment operator.
        /// @param other The other `fenced_pointer<T>` to copy.
        /// @return Reference to `*this`.
        inline fenced_pointer& operator=(const fenced_pointer& other) noexcept
        {
            this->value = other.value;
            return *this;
        }

        /// @brief Obtain the pointer value, without fencing it against `nullptr`.
        /// @return The underlying pointer value.
        /// @note This function is marked unchecked because it will not throw if `this->value == nullptr`.
        inline T* unchecked() const noexcept
        {
            return this->value;
        }
        /// @brief Obtain the pointer value, throwing if `nullptr`.
        /// @return The underlying pointer value.
        [[nodiscard]] inline T& operator*() const
        {
            return *this->obtain_or_throw();
        }
        /// @brief Obtain the pointer value, throwing if `nullptr`.
        /// @return The underlying pointer value.
        [[nodiscard]] inline T* operator->() const
        {
            return this->obtain_or_throw();
        }
        /// @brief Obtain value at index, throwing if `nullptr`.
        /// @param index The index to access.
        /// @return The value at the index.
        [[nodiscard]] inline T& operator[](std::size_t index) const
        {
            return this->obtain_or_throw()[index];
        }

        /// @brief Spaceship operator.
        /// @param other The other `fenced_pointer<T>` to compare against.
        /// @return The comparison result.
        inline auto operator<=>(const fenced_pointer& other) const noexcept = default;

        /// @brief Prefix increment operator.
        /// @return Reference to `*this`.
        inline fenced_pointer& operator++() noexcept
        {
            _push_nowarn(_clWarn_pedantic);
            void* jmp[2] { &&ThrowNull, &&Op };
            goto* jmp[bool(this->value)];
            _pop_nowarn();
        ThrowNull:
            _throw(null_pointer_exception());
        Op:
            ++this->value;
            return *this;
        }
        /// @brief Prefix decrement operator.
        /// @return Reference to `*this`.
        inline fenced_pointer& operator--() noexcept
        {
            _push_nowarn(_clWarn_pedantic);
            void* jmp[2] { &&ThrowNull, &&Op };
            goto* jmp[bool(this->value)];
            _pop_nowarn();
        ThrowNull:
            _throw(null_pointer_exception());
        Op:
            --this->value;
            return *this;
        }
        /// @brief Postfix increment operator.
        /// @return The value before incrementing.
        inline fenced_pointer operator++(int) noexcept
        {
            _push_nowarn(_clWarn_pedantic);
            void* jmp[2] { &&ThrowNull, &&Op };
            goto* jmp[bool(this->value)];
            _pop_nowarn();
        ThrowNull:
            _throw(null_pointer_exception());
        Op:
            fenced_pointer ret = *this;
            ++this->value;
            return ret;
        }
        /// @brief Postfix decrement operator.
        /// @return The value before decrementing.
        inline fenced_pointer operator--(int) noexcept
        {
            _push_nowarn(_clWarn_pedantic);
            void* jmp[2] { &&ThrowNull, &&Op };
            goto* jmp[bool(this->value)];
            _pop_nowarn();
        ThrowNull:
            _throw(null_pointer_exception());
        Op:
            fenced_pointer ret = *this;
            --this->value;
            return ret;
        }
        /// @brief Addition operator.
        /// @param offset The offset to add.
        /// @return The pointer with the offset added.
        inline fenced_pointer operator+(ptrdiff_t offset) const noexcept
        {
            return fenced_pointer { this->value + offset };
        }
        /// @brief Subtraction operator.
        /// @param offset The offset to subtract.
        /// @return The pointer with the offset subtracted.
        inline fenced_pointer operator-(ptrdiff_t offset) const noexcept
        {
            return fenced_pointer { this->value - offset };
        }
        /// @brief Addition operator.
        /// @param offset The offset to add.
        /// @return The pointer with the offset added.
        inline ptrdiff_t operator-(const fenced_pointer& other) const noexcept
        {
            return this->value - other.value;
        }
        /// @brief Addition assignment operator.
        /// @param offset The offset to add.
        /// @return Reference to `*this`.
        inline fenced_pointer& operator+=(ptrdiff_t offset) noexcept
        {
            _push_nowarn(_clWarn_pedantic);
            void* jmp[2] { &&ThrowNull, &&Op };
            goto* jmp[bool(this->value)];
            _pop_nowarn();
        ThrowNull:
            _throw(null_pointer_exception());
        Op:
            this->value += offset;
            return *this;
        }
        /// @brief Subtraction assignment operator.
        /// @param offset The offset to subtract.
        /// @return Reference to `*this`.
        inline fenced_pointer& operator-=(ptrdiff_t offset) noexcept
        {
            _push_nowarn(_clWarn_pedantic);
            void* jmp[2] { &&ThrowNull, &&Op };
            goto* jmp[bool(this->value)];
            _pop_nowarn();
        ThrowNull:
            _throw(null_pointer_exception());
        Op:
            this->value -= offset;
            return *this;
        }
    private:
        T* value = nullptr;

        /// @brief Obtain the pointer value, throwing if `nullptr`.
        /// @return The underlying pointer value.
        [[nodiscard]] inline T* obtain_or_throw() const
        {
            _push_nowarn(_clWarn_pedantic);
            void* jmp[2] { &&ThrowNull, &&Op };
            goto* jmp[bool(this->value)];
            _pop_nowarn();
        ThrowNull:
            _throw(null_pointer_exception());
        Op:
            return this->value;
        }
    };

#if !defined(__clang__) && defined(__GNUC__)
    inline std::unique_ptr<char, void (*)(void*)> exception_type_name(std::exception_ptr ex)
    {
        return std::unique_ptr<char, void (*)(void*)>(abi::__cxa_demangle(ex.__cxa_exception_type()->name() /* Contract implied: `name != nullptr`. */, NULL, NULL, NULL),
                                                      std::free);
    }
#else
    inline std::unique_ptr<const char, void (*)(const void*)> exception_type_name([[maybe_unused]] std::exception_ptr ex)
    {
        return std::unique_ptr<const char, void (*)(const void* const)>("<unknown>", [](const void* const) { });
    }
#endif

    /// @brief Terminate the program.
    /// @note Pass `byref`.
    struct terminate_exception : public managed_exception
    {
        /// @brief On construction, terminate the program.
        inline terminate_exception() noexcept
        {
            std::terminate();
        }

        /// @brief Never called.
        /// @return [[noreturn]]
        [[noreturn]] constexpr const char* what() const noexcept override
        {
            std::unreachable();
        }
    };
    /// @brief Thrown when a contract is violated.
    /// @note
    /// Prefer using the `_fence_contract_enforce` macro instead. Don't catch this exception.
    /// Pass `byref`.
    struct contract_violation_exception : public managed_exception
    {
        /// @brief Construct the exception with a reason, which is logged on construction.
        /// @param why The reason the contract was violated.
        template <size_t N>
        inline contract_violation_exception(const char (&why)[N]) noexcept : why(why)
        {
            std::println(stderr, "Note: {}", why);
        }

        /// @brief What went wrong.
        /// @return The reason a contract was violated.
        inline const char* what() const noexcept override
        {
            return this->why;
        }
    private:
        const char* why;
    };
} // namespace sys