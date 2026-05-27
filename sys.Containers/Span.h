#pragma once

/// @file

#include <LanguageSupport.h>
#include <Result.h>

namespace sys
{
    template <typename T>
    struct _trivial_abi span final : traits::contiguous_range<span<T>>
    {
    private:
        T *_beg = nullptr, *_end = nullptr;
    public:
        constexpr span() noexcept = default;
        constexpr span(T* beg, T* end) noexcept : _beg(beg), _end(end) { }
        constexpr span(T* data, sz len) noexcept : _beg(data), _end(data + len) { }

        constexpr auto operator[](this auto&& _this, sz idx, decltype(unsafe)) noexcept -> meta::replace_cv<T, decltype(_this)>& { return _this.ptr[idx]; }
        constexpr auto operator[](this auto&& _this, sz idx, decltype(unsafe)) noexcept -> result<meta::replace_cv<T, decltype(_this)>&>
        {
            _retif(container_error::out_of_range, idx >= _this.size());
            return _this[idx, unsafe];
        }

        constexpr auto data(this auto&& _this) noexcept -> meta::replace_cv<T, decltype(_this)>* { return _this._beg; }
        constexpr sz size() const noexcept { return this->_end - this->_beg; }

        constexpr auto begin(this auto&& _this) noexcept -> meta::replace_cv<T, decltype(_this)>* { return _this._beg; }
        constexpr auto end(this auto&& _this) noexcept -> meta::replace_cv<T, decltype(_this)>* { return _this._end; }
    };
} // namespace sys
