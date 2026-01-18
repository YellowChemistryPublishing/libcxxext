#pragma once

#include <cstddef>
#include <cstring>

namespace sys
{
    template <size_t Capacity, typename CharType = char>
    class InplaceString
    {
        CharType buffer[Capacity + 1] {};
        size_t _size;
    public:
        constexpr InplaceString() : _size(0)
        {
            this->buffer[0] = 0;
        }
        constexpr InplaceString(const char* cstr) : _size(0) // NOLINT(hicpp-explicit-conversions)
        {
            const size_t len = std::strlen(cstr);
            if (len > Capacity)
            {
                this->buffer[0] = 0;
                return;
            }

            memcpy(this->buffer, cstr, len + 1);
        }

        [[nodiscard]] CharType* begin()
        {
            return this->buffer;
        }
        [[nodiscard]] CharType* end()
        {
            return this->buffer + this->_size;
        }
    };
} // namespace sys
