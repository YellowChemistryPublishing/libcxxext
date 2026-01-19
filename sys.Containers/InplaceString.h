#pragma once

#include <cstddef>
#include <cstring>

namespace sys
{
    template <size_t Capacity, typename CharType = char>
    class inplace_string
    {
        CharType buffer[Capacity + 1] {};
        size_t _size;
    public:
        constexpr inplace_string() : _size(0) { this->buffer[0] = 0; }
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

        [[nodiscard]] CharType* begin() { return this->buffer; }
        [[nodiscard]] CharType* end() { return this->buffer + this->_size; }
    };
} // namespace sys
