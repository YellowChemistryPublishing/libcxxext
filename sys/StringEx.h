#pragma once

#include <algorithm>
#include <concepts>
#include <cxxsup.h>
#include <functional>
#include <span>
#include <string_view>

#include <LanguageSupport.h>
#include <ManagedArray.h>

namespace sys
{
    template <std::integral CharType = char8_t, bool DynamicExtent = true, ssz StaticCapacity = 4>
    requires (StaticCapacity > 0 && std::is_trivially_constructible_v<CharType> && std::is_trivially_copyable_v<CharType> &&
              requires(CharType c) {
                  { c == 0 } -> std::convertible_to<bool>;
              })
    struct string
    {
        template <IEnumerable<string> Container>
        _const static result<string> concat(Container&& container)
        {
            ssz totalLength = 0;
            for (auto& str : container) totalLength += str.length();

            string ret;
            auto to = ret.alloc(nr2i64(totalLength + 1zz));
            if (!to)
                return nullptr;

            for (auto& str : container)
            {
                std::copy(str.begin(), str.end(), to);
                to += str.length();
            }
            *to = 0;

            ret._length = totalLength;
            return ret;
        }

        inline string()
        {
            this->dataStatic[0] = 0;
        }
        inline string(CharType c, ssz repeat)
        {
            this->for_each_assign(repeat, [&](CharType* buf, ssz i) { buf[i] = c; }, [&, this] { return this->alloc(nr2i64(repeat + 1zz)); });
        }
        inline string(const std::span<CharType> str)
        {
            this->for_each_assign(str.size(), [&](CharType* buf, ssz i) { buf[i] = str[i]; }, [&, this] { return this->alloc(nr2i64(str.size() + 1)); });
        }
        inline string(const CharType* str)
        {
            ssz len = 0;
            while (str[len]) ++len;
            this->for_each_assign(len, [&](CharType* buf, ssz i) { buf[i] = str[i]; }, [&, this] { return this->alloc(nr2i64(len + 1zz)); });
        }
        template <bool OtherDynamicExtent, ssz OtherStaticCapacity>
        inline string(const string<CharType, OtherDynamicExtent, OtherStaticCapacity>&)
        {
            _assert_ctor_can_fail();
        }
        template <bool OtherDynamicExtent, ssz OtherStaticCapacity>
        inline static result<string<CharType, DynamicExtent, StaticCapacity>> ctor(const string<CharType, OtherDynamicExtent, OtherStaticCapacity>& other)
        {
            if (other.is_dynamic())
            {
                string<CharType, DynamicExtent, StaticCapacity> ret;
                ret->for_each_assign(other._length, [&](CharType* buf, ssz i) { buf[i] = other.dataDynamic.buf[i]; }, [&]
                {
                    ret->dataDynamic.buf = ManagedArray<CharType>(other.dataDynamic.capacity);
                    ret->dataDynamic.capacity = other.dataDynamic.capacity;
                    ret->_isDynamic = true;
                    return ret->dataDynamic.buf.begin();
                });
            }
            else if (other._length < StaticCapacity)
            {
                string<CharType, DynamicExtent, StaticCapacity> ret;
                ret.for_each_assign(other._length, [&](CharType* buf, ssz i) { buf[i] = other.dataStatic[i]; }, [&]
                {
                    ret._isDynamic = false;
                    return ret.dataStatic;
                });
            }
            else
                return nullptr;
        }
        inline string(string<CharType, DynamicExtent, StaticCapacity>&& other)
        {
            swap(*this, other);
        }
        inline ~string()
        {
            this->dealloc();
        }

        inline operator std::basic_string_view<CharType>() const noexcept
        {
            if (this->is_dynamic())
                return std::basic_string_view<CharType>(this->dataDynamic.buf.begin(), this->_length);
            else
                return std::basic_string_view<CharType>(this->dataStatic, this->_length);
        }

        friend inline auto operator<=>(const string& a, const string& b)
        {
            return std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end());
        }

        inline CharType& operator[](ssz i, unsafe) noexcept
        {
            if (this->is_dynamic())
                return this->dataDynamic.buf[i];
            else
                return this->dataStatic[i];
        }
        inline CharType operator[](ssz i, unsafe) const noexcept
        {
            if (this->is_dynamic())
                return this->dataDynamic.buf[i];
            else
                return this->dataStatic[i];
        }
        inline result<CharType&> operator[](ssz i)
        {
            if (ssz(0) <= i && i < this->_length)
                return (*this)[i, unsafe()];
            else
                return nullptr;
        }
        inline result<CharType> operator[](ssz i) const
        {
            if (ssz(0) <= i && i < this->_length)
                return (*this)[i, unsafe()];
            else
                return nullptr;
        }

        inline const CharType* cbegin() const noexcept
        {
            if (this->is_dynamic())
                return this->dataDynamic.buf.begin();
            else
                return this->dataStatic;
        }
        inline const CharType* cend() const noexcept
        {
            if (this->is_dynamic())
                return this->dataDynamic.buf.end();
            else
                return this->dataStatic + this->_length;
        }
        inline const CharType* begin() const noexcept
        {
            return this->cbegin();
        }
        inline const CharType* end() const noexcept
        {
            return this->cend();
        }
        inline CharType* begin() noexcept
        {
            if (this->is_dynamic())
                return this->dataDynamic.buf.begin();
            else
                return this->dataStatic;
        }
        inline CharType* end() noexcept
        {
            if (this->is_dynamic())
                return this->dataDynamic.buf.end();
            else
                return this->dataStatic + this->_length;
        }

        constexpr bool is_dynamic() const noexcept
        {
            if constexpr (DynamicExtent)
                return this->_isDynamic;
            else
                return false;
        }
        inline bool is_empty() const
        {
            return this->_length == 0;
        }
        inline ssz length() const
        {
            return this->_length;
        }
        inline size_t size() const
        {
            return _asi(size_t, this->length());
        }
        inline ssz capacity() const
        {
            if (this->is_dynamic())
                return this->dataDynamic.capacity;
            else
                return StaticCapacity;
        }

        inline bool contains(CharType c) const
        {
            return this->find(c);
        }
        inline bool contains(const std::basic_string_view<CharType> str) const
        {
            return this->find(str);
        }
        inline bool starts_with(const std::basic_string_view<CharType> str) const
        {
            if (this->length() < str.length())
                return false;
            return std::equal(this->cbegin(), this->cbegin() + str.length(), str.begin());
        }
        inline bool ends_width(const std::basic_string_view<CharType> str) const
        {
            if (this->length() < str.length())
                return false;
            return std::equal(this->cend() - str.length(), this->cend(), str.begin());
        }
        inline const CharType* find(CharType c) const
        {
            for (auto it = this->cbegin(); it != this->cend(); ++it)
            {
                if (*it == c)
                    return it;
            }
            return nullptr;
        }
        inline const CharType* find(const std::basic_string_view<CharType> str) const
        {
            for (auto it = this->cbegin(); it != this->cend() - str.length() + 1; ++it)
            {
                if (std::equal(it, it + str.length(), str.begin()))
                    return it;
            }
            return nullptr;
        }
        inline CharType* find(CharType c)
        {
            for (auto it = this->begin(); it != this->end(); ++it)
            {
                if (*it == c)
                    return it;
            }
            return nullptr;
        }

        inline sz hash_code()
        {
            return std::hash(_as(std::basic_string_view<CharType>, *this))();
        }
        friend inline void swap(sys::string<CharType, DynamicExtent, StaticCapacity>& a, sys::string<CharType, DynamicExtent, StaticCapacity>& b) noexcept
        {
            using std::swap;

            if (&a != &b) [[likely]]
                std::swap_ranges(_asr(byte*, &a), _asr(byte*, &a) + sizeof(a), _asr(byte*, &b));
        }
    private:
        union
        {
            struct
            {
                ManagedArray<CharType> buf;
                ssz capacity = 0;
            } dataDynamic;
            CharType dataStatic[+sz(StaticCapacity)];
        };
        bool _isDynamic = false;
        ssz _length = 0;

        inline string(unsafe)
        { }

        [[nodiscard]] inline CharType* alloc(ssz capacity)
        {
            if constexpr (DynamicExtent)
            {
                if (capacity > StaticCapacity)
                {
                    this->dataDynamic.buf = ManagedArray<CharType>(capacity);
                    this->dataDynamic.capacity = capacity;
                    this->_isDynamic = true;
                    return this->dataDynamic.buf.begin();
                }
                else
                {
                    this->_isDynamic = false;
                    return this->dataStatic;
                }
            }

            if (capacity <= StaticCapacity)
                return this->dataStatic;

            return nullptr;
        }
        inline void dealloc()
        {
            if (this->is_dynamic())
                this->dataDynamic.buf.~ManagedArray();
        }
        inline bool for_each_assign(ssz len, auto&& withIndex, auto&& createFetchBuffer)
        {
            if (CharType* buf = std::invoke_r<CharType*>(createFetchBuffer))
            {
                for (ssz i = 0; i < len; i++) std::invoke(withIndex, buf, i);
                buf[len] = 0;
                this->_length = len;
                return true;
            }
            return false;
        }
    };
} // namespace sys

using str8 = sys::string<char8_t>;
using str16 = sys::string<char16_t>;
using str32 = sys::string<char32_t>;
using str = sys::string<char>;
using wstr = sys::string<wchar_t>;
