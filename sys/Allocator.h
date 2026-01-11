#pragma once

#include <cstring>
#include <new>

#include <LanguageSupport.h>

namespace sys
{
    /// @defgroup except_allocator
    /// @throw `std::bad_alloc` (`sys::allocator<...>::allocate(...)`)

    template <typename T, i16 StaticSize = 0_i16, bool DynamicExtents = true>
    requires (StaticSize >= 0)
    class allocator
    {
        union
        {
            byte _;
            T inplaceBuffer[size_t(+StaticSize)];
        };
        byte bufferUnavail[size_t(+StaticSize)] {};
    public:
        using size_type = ssz;
        using pointer = T*;
        using const_pointer = const T*;

        constexpr allocator() noexcept
        { }

        /// @copydetail except_operator_new
        /// @copydetail except_allocator
        inline T* allocate(size_t n)
        {
            auto beg = -1_i16;
            for (auto end = 0_i16; end < StaticSize; end++)
            {
                if (beg < 0_i16)
                    beg = end;
                else if (end - beg >= i16(n))
                {
                    std::memset(&this->bufferUnavail[+beg], 1u, n);
                    return &this->inplaceBuffer[+beg];
                }
                else if (bufferUnavail[+end] == 1u)
                    beg = -1_i16;
            }
            if constexpr (DynamicExtents)
                return _as(T*, ::operator new(n));
            else
                _throw(std::bad_alloc());
        }
        inline void deallocate(T* p, size_t n) noexcept
        {
            if constexpr (DynamicExtents)
            {
                if (this->inplaceBuffer <= p && p < this->inplaceBuffer + (+StaticSize))
                    std::memset(this->bufferUnavail + (p - this->inplaceBuffer), 0u, n);
                else
                    ::operator delete(p); // NOLINT: We mean to delete `p`.
            }
            else
                std::memset(this->bufferUnavail + (p - this->inplaceBuffer), 0u, n);
        }
    };

    template <typename T>
    class allocator<T, 0_i16, true>
    {
    public:
        using size_type = ssz;
        using pointer = T*;
        using const_pointer = const T*;

        /// @copydetail except_operator_new
        /// @copydetail except_allocator
        inline T* allocate(size_t n)
        {
            return _as(T*, ::operator new(n));
        }
        inline void deallocate(T* p, size_t) noexcept
        {
            ::operator delete(p);
        }
    };

    template <typename T>
    class allocator<T, 0_i16, false>
    {
    public:
        static_assert(false, "This type doesn't make any sense!");
    };

    template <typename T, i16 StaticSize>
    using inplace_allocator = allocator<T, StaticSize, false>;
} // namespace sys
