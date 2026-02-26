#pragma once

#include <cstddef>
#include <cstring>

namespace sys
{
    /// @brief Inplace string of fixed capacity.
    template <size_t Capacity, typename CharType = char>
    class inplace_string
    {
        CharType buffer[Capacity + 1] {};
        size_t _size;
    public:
        /// @brief Constructs an empty string.
        constexpr inplace_string() : _size(0) { this->buffer[0] = 0; }
        /// @brief Constructs a string from a C-string.
        constexpr inplace_string(const char* cstr) : _size(0) // NOLINT(hicpp-explicit-conversions)
        {
            const size_t len = std::strlen(cstr);
            if (len > Capacity)
            {
                this->buffer[0] = 0;
                return;
            }

            memcpy(this->buffer, cstr, len + 1);
        }

        /// @brief Pointer to the beginning of the string.
        [[nodiscard]] CharType* begin() { return this->buffer; }
        /// @brief Pointer to the end of the string.
        [[nodiscard]] CharType* end() { return this->buffer + this->_size; }
    };
} // namespace sys
