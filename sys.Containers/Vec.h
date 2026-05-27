#pragma once

/// @file

#include <Aggregate.h>
#include <Alloc.h>
#include <BitTwiddling.h>
#include <ContainerErrors.h>
#include <Destructor.h>
#include <Lifetime.h>
#include <Ranges.h>
#include <Result.h>
#include <Span.h>
#include <meta/Function.h>
#include <meta/InterfaceRequirements.h>
#include <meta/NamedRequirements.h>
#include <meta/Type.h>
#include <traits/Ranges.h>

namespace sys
{
    /// @brief A resizable array-like type.
    /// @tparam T
    /// @tparam Alloc
    template <typename T, IAllocator<T> Alloc = small_buffer_allocator<T>>
    requires (meta::type<T>::is_unqualified())
    class vec final : protected Alloc, traits::contiguous_range<vec<T, Alloc>>
    {
        T* buf = nullptr;
        sz cap = 0_uz, len = 0_uz;

        /// @internal
        /// @note If reallocation fails, the original buffer is valid, but may have moved-from elements which are allowed to be in an unspecified state.
        [[nodiscard]] constexpr container_error realloc_cap_unless_zero(const sz newCap, decltype(unsafe)) noexcept(INothrowMoveConstructible<T> && INothrowDestructible<T>)
        {
            _retif(container_error::overflow, newCap == 0_uz); // Bad new capacity.

            T* newBuf = nullptr;
            if (ITriviallyRelocatable<T> || this->will_realloc_inplace(this->buf, this->cap, newCap, unsafe))
            {
                newBuf = this->realloc(this->buf, this->cap, newCap, unsafe);
                _retif(container_error::oom, !newBuf);
            }
            else if constexpr (INothrowMoveConstructible<T> && INothrowDestructible<T>)
            {
                newBuf = this->alloc(newCap, unsafe);
                _retif(container_error::oom, !newBuf);

                for (T *it = newBuf, *oldIt = this->buf; oldIt < this->buf + this->len; ++it, ++oldIt)
                {
                    if (it < newBuf + newCap)
                        construct_at(it, std::move(*oldIt));
                    destroy_at(oldIt);
                }
                this->dealloc(this->buf, this->cap, unsafe);
            }
            else
            {
                newBuf = this->alloc(newCap, unsafe);
                _retif(container_error::oom, !newBuf);

                T* movedUpTo = newBuf;
                optional_destructor releaseNewBuf = [this, &newBuf, &newCap, &movedUpTo]() noexcept -> void
                {
                    if constexpr (!ITriviallyDestructible<T>)
                        for (T* it = newBuf; it < movedUpTo; ++it)
                            destroy_at(it);
                    this->dealloc(newBuf, newCap, unsafe);
                };
                for (T *it = newBuf, *oldIt = this->buf; it < newBuf + newCap && oldIt < this->buf + this->len; ++it, ++oldIt)
                {
                    construct_at(it, std::move(*oldIt));
                    movedUpTo = it + 1z;
                }
                if constexpr (!ITriviallyDestructible<T>)
                    for (T* oldIt = this->buf; oldIt < this->buf + this->len; ++oldIt)
                        destroy_at(oldIt);

                releaseNewBuf.clear();
                this->dealloc(this->buf, this->cap, unsafe);
            }

            this->buf = newBuf;
            this->cap = newCap;
            this->len = std::min(this->len, newCap);
            return container_error::ok;
        }

        /// @internal
        /// @brief Optimized transfer from any enumerable.
        constexpr container_error transfer_from(IEnumerable<T> auto&& other, decltype(unsafe)) noexcept(INothrowConstructibleFrom<T, meta::replace_cv<T, decltype(other)>> &&
                                                                                                        INothrowAssignableFrom<T, meta::replace_cv<T, decltype(other)>> &&
                                                                                                        INothrowMoveConstructible<T> && INothrowDestructible<T>)
        {
            if (this->cap >= other.size())
            {
                T *it = this->buf, *itFrom = other.buf;
                for (it, itFrom; it < this->buf + this->len; ++it, ++itFrom)
                    *it = std::forward_like<T>(*itFrom);
                while (this->len < other.len)
                {
                    construct_at(it++, std::forward_like<T>(*(itFrom++)));
                    ++this->len;
                }
            }
            else
            {
                this->dealloc_buf();

                if (other.cap <= 0_uz)
                    return container_error::ok;
                _retif(err, auto err = this->realloc_cap_unless_zero(other.cap); err != container_error::ok);

                for (T *it = this->buf, *itFrom = other.buf; itFrom < other.buf + other.len; ++it, ++itFrom)
                    construct_at(it, std::forward_like<T>(*itFrom));
            }

            return container_error::ok;
        }

        /// @internal
        /// @brief Optimized move from another `vec<...>`.
        constexpr container_error move_from(vec&& other, decltype(unsafe)) noexcept(INothrowMoveConstructible<T> && INothrowMoveAssignable<T> && INothrowDestructible<T>)
        {
            if (this->is_stateless(this->buf) && other.is_stateless(other.buf))
            {
                using std::swap;

                swap(this->buf, other.buf);
                swap(this->len, other.len);
                swap(this->cap, other.cap);

                return container_error::ok;
            }

            return this->transfer_from(std::move(other), unsafe);
        }

        /// @internal
        /// @brief Destroy up to `this->len` elements, in descending address-order, and deallocate the buffer.
        constexpr void dealloc_buf(decltype(unsafe)) noexcept(INothrowDestructible<T>)
        {
            if constexpr (!ITriviallyDestructible<T>)
            {
                while (this->len > 0_uz)
                    std::destroy_at(this->buf + --this->len);
            }
            else
                this->len = 0_uz;

            this->dealloc(this->buf, this->cap, unsafe);
            this->buf = nullptr;
            this->cap = 0_uz;
        }
    public:
        using value_type = T;
        using allocator_type = Alloc;

        /// @brief The number of elements storable when initially reserving a buffer, unless otherwise specified.
        static consteval sz default_initial_capacity() noexcept
        {
            if (sizeof(T) <= 1uz)
                return 8_uz;
            else if (sizeof(T) <= 1024uz)
                return 4_uz;
            else
                return 1_uz;
        }

        constexpr vec() noexcept(INothrowDefaultConstructible<Alloc>) = default;
        constexpr vec(Alloc alloc) noexcept(INothrowMoveConstructible<Alloc>) : Alloc(std::move(alloc)) { }

        constexpr vec(T* ptr, sz len, sz cap, decltype(unsafe)) noexcept(INothrowDefaultConstructible<Alloc>) : ptr(ptr), len(len), cap(cap) { }
        constexpr vec(T* ptr, sz len, sz cap, Alloc alloc, decltype(unsafe)) noexcept(INothrowMoveConstructible<Alloc>) : Alloc(std::move(alloc)), ptr(ptr), len(len), cap(cap) { }

        constexpr vec(const vec& other) noexcept = delete;
        constexpr vec(vec&& other)
        {
            _contract_assert(this->move_from(std::move(other)) == container_error::ok,
                             "If your allocator is too stateful, and thus can't be adequately used in move construction, you should use a different one!");
        }

        constexpr ~vec() noexcept(INothrowDestructible<T>) { this->dealloc_buf(); }

        constexpr vec& operator=(const vec& other) noexcept = delete;
        constexpr vec& operator=(vec&& other)
        {
            _retif(*this, this == &other);

            _contract_assert(this->move_from(std::move(other)) == container_error::ok,
                             "If your allocator is too stateful, and thus can't be adequately used in move assignment, you should use a different one!");
            return *this;
        }

        static constexpr auto /* result<vec, container_error> */ ctor(const vec& other) noexcept(INothrowCopyConstructible<T>)
        {
            vec ret;
            _retif((result<vec, container_error>(res.err())), auto res = ret.transfer_from(other, unsafe); !res);
            return result<vec, container_error>(std::move(ret));
        }
        static constexpr auto /* result<vec, container_error> */ ctor(vec&& other) noexcept(INothrowMoveConstructible<T>)
        {
            vec ret;
            _retif((result<vec, container_error>(res.err())), auto res = ret.move_from(std::move(other)); !res);
            return result<vec, container_error>(std::move(ret));
        }
        static constexpr auto /* result<vec, container_error> */ ctor(std::initializer_list<T> il)
        {
            vec ret;
            _retif((result<vec, container_error>(res.err())), auto res = ret.transfer_from(il, unsafe); !res);
            return result<vec, container_error>(std::move(ret));
        }
        static constexpr auto /* result<vec, container_error> */ ctor(sz count, const T& val = T()) noexcept(INothrowCopyConstructible<T>)
        {
            vec ret;
            if (count <= 0_uz)
                return result<vec, container_error>(std::move(ret));
            _retif((result<vec, container_error>(res.err())), auto res = ret.realloc_cap_unless_zero(count); res != container_error::ok);

            for (T* it = ret.buf; it < ret.buf + count; ++it)
            {
                construct_at(it, val);
                ++ret.len;
            }
            return result<vec, container_error>(std::move(ret));
        }

        static constexpr auto /* result<vec, container_error> */ ctor_with_capacity(sz cap, Alloc alloc = Alloc()) noexcept(INothrowMoveConstructible<Alloc>)
        {
            vec ret(std::move(alloc));
            if (cap <= 0_uz)
                return result<vec, container_error>(std::move(ret));
            _retif((result<vec, container_error>(res.err())), auto res = ret.realloc_cap_unless_zero(cap); res != container_error::ok);
            return result<vec, container_error>(std::move(ret));
        }
        static constexpr auto /* result<vec, container_error> */ ctor_with_at_least_capacity(sz capAtLeast, Alloc alloc = Alloc()) noexcept(INothrowMoveConstructible<Alloc>)
        {
            return vec::ctor_with_capacity(std::max(bit_ceil(capAtLeast), capAtLeast), std::move(alloc));
        }

        static constexpr auto /* result<vec, container_error> */ ctor(sz len,
                                                                      IFunc<T(sz)> auto&& generator) noexcept(INothrowCallable<decltype(generator)> && INothrowMoveConstructible<T>)
        {
            vec ret;
            if (len <= 0_uz)
                return result<vec, container_error>(std::move(ret));
            _retif((result<vec, container_error>(res.err())), auto res = ret.realloc_cap_unless_zero(len); res != container_error::ok);

            for (T* it = ret.buf; it < ret.buf + len; ++it)
            {
                construct_at(it, generator(it - ret.buf));
                ++ret.len;
            }
            return result<vec, container_error>(std::move(ret));
        }

        constexpr result<vec&, container_error> assign_from(IEnumerable<T> auto&& other)
        {
            if constexpr (std::same_as<vec, _decltype_of(other)>)
                _retif(container_error::ok, this == &other);
            _retif((result<vec&, container_error>(err)), container_error err = this->transfer_from(_forward(other), unsafe); err != container_error::ok);
            return result<vec&, container_error>(*this);
        }
        constexpr result<vec&, container_error> assign_from(vec&& other)
        {
            _retif(container_error::ok, this == &other);
            return this->move_from(std::move(other));
        }

        constexpr bool empty() const noexcept { return this->len <= 0_uz; }
        constexpr sz size() const noexcept { return this->len; }
        constexpr sz capacity() const noexcept { return this->cap; }
        constexpr auto data(this auto&& _this) noexcept -> meta::forwarded_like<T, decltype(_this)>* { return _as(_this.buf, meta::forwarded_like<T, decltype(_this)>*); }

        constexpr auto operator[](this auto&& _this, sz idx, decltype(unsafe)) -> meta::forwarded_like<T, decltype(_this)>&
        {
            return std::forward_like<decltype(_this)>(*(_this.buf + idx));
        }
        constexpr auto operator[](this auto&& _this, sz idx) -> result<meta::forwarded_like<T, decltype(_this)>&, container_error>
        {
            _retif(container_error::out_of_range, idx >= this->len);
            return (*this)[idx, unsafe];
        }

        constexpr auto front(this auto&& _this, decltype(unsafe)) -> meta::forwarded_like<T, decltype(_this)>& { return std::forward_like<decltype(_this)>(*this->buf); }
        constexpr auto front(this auto&& _this) -> result<meta::forwarded_like<T, decltype(_this)>&, container_error>
        {
            _retif(container_error::out_of_range, this->empty());
            return this->front(unsafe);
        }
        constexpr auto back(this auto&& _this, decltype(unsafe)) -> meta::forwarded_like<T, decltype(_this)>&
        {
            return std::forward_like<decltype(_this)>(*(this->buf + *sz(ssz(this->len) - 1_z)));
        }
        constexpr auto back(this auto&& _this) -> result<meta::forwarded_like<T, decltype(_this)>&, container_error>
        {
            _retif(container_error::out_of_range, this->empty());
            return this->back(unsafe);
        }

        constexpr span<const T> as_slice() const noexcept { return { this->buf, this->buf + this->len }; }
        constexpr span<T> as_view() noexcept { return { this->buf, this->buf + this->len }; }

        constexpr subrange<T*> range(this auto&& _this) noexcept { return { this->buf, this->buf + this->len }; }

        constexpr aggregate<T*, sz, sz> /* ptr, len, cap, alloc */ raw() && noexcept
        {
            return { std::exchange(this->buf, nullptr), std::exchange(this->len, 0_uz), std::exchange(this->cap, 0_uz) };
        }
        constexpr aggregate<T*, sz, sz, Alloc> /* ptr, len, cap, alloc */ raw_with_allocator() && noexcept(INothrowMoveConstructible<Alloc>)
        {
            return { std::exchange(this->buf, nullptr), std::exchange(this->len, 0_uz), std::exchange(this->cap, 0_uz), std::move(_as(*this, Alloc&)) };
        }
        constexpr auto allocator(this auto&& _this) noexcept -> meta::forwarded_like<Alloc, decltype(_this)> { return std::forward_like<Alloc&>(_this); }
        constexpr span<T> spare_capacity() noexcept { return { this->buf + this->len, this->buf + this->cap }; }

        constexpr result<void, container_error> reserve_exactly(sz newCap) noexcept(INothrowMoveConstructible<T> && INothrowDestructible<T>)
        {
            _retif((result<void, container_error>()), newCap <= this->cap || newCap <= 0_uz);
            return this->realloc_cap_unless_zero(newCap, unsafe);
        }
        constexpr result<void, container_error> reserve(sz newCap) noexcept(INothrowMoveConstructible<T> && INothrowDestructible<T>)
        {
            return this->reserve_exactly(std::max(bit_ceil(newCap), newCap));
        }
        constexpr result<void, container_error> shrink_to(sz atLeastCap) noexcept(INothrowMoveConstructible<T> && INothrowDestructible<T>)
        {
            _retif(container_error::ok, atLeastCap <= this->len || this->cap <= 0_uz);
            return this->realloc_cap_unless_zero(atLeastCap, unsafe);
        }
        constexpr result<void, container_error> shrink_to_fit() noexcept(INothrowMoveConstructible<T> && INothrowDestructible<T>) { return this->shrink_to(this->len); }

        constexpr result<void, container_error> resize(sz newLen, const T& val = T()) noexcept(INothrowMoveConstructible<T> && INothrowDestructible<T>)
        {
            if (newLen <= this->len)
                return this->truncate(newLen);

            if (newLen > this->cap)
                _retif((result<void, container_error>(err)), container_error err = this->realloc_cap_unless_zero(std::max(bit_ceil(newLen), newLen), unsafe);
                       err != container_error::ok);

            for (T* it = this->buf + this->len; it < this->buf + newLen; ++it)
            {
                construct_at(it, val);
                if constexpr (!INothrowCopyConstructible<T>)
                    ++this->len;
            }
            if constexpr (INothrowCopyConstructible<T>)
                this->len = newLen;

            return result<void, container_error>();
        }
        constexpr result<void, container_error> resize_with(sz newLen, IFunc<T()> auto&& generator) noexcept
        {
            if (newLen <= this->len)
                return this->truncate(newLen);

            if (newLen > this->cap)
                _retif((result<void, container_error>(err)), container_error err = this->realloc_cap_unless_zero(std::max(bit_ceil(newLen), newLen), unsafe);
                       err != container_error::ok);

            for (T* it = this->buf + this->len; it < this->buf + newLen; ++it)
            {
                construct_at(it, std::invoke(generator));
                if constexpr (!INothrowMoveConstructible<T>)
                    ++this->len;
            }
            if constexpr (INothrowMoveConstructible<T>)
                this->len = newLen;

            return result<void, container_error>();
        }

        constexpr result<void, container_error> push(auto&&... args) noexcept(INothrowConstructibleFrom<T, decltype(_forward(args))...> && INothrowMoveConstructible<T> &&
                                                                              INothrowDestructible<T>)
        {
            if (this->len == this->cap)
                _retif(err, container_error err = this->realloc_cap_unless_zero(this->cap > 0_uz ? this->cap * 2_uz : vec::default_initial_capacity(), unsafe);
                       err != container_error::ok);

            construct_at(this->buf + this->len, _forward(args)...);
            ++this->len;
            return {};
        }
        constexpr result<T&, container_error> push_fetch(auto&&... args) noexcept(INothrowConstructibleFrom<T, decltype(_forward(args))...> && INothrowMoveConstructible<T> &&
                                                                                  INothrowDestructible<T>)
        {
            _retif(res.err(), auto res = this->push(_forward(args)...); !res);
            return (*this)[this->len - 1_uz, unsafe];
        }

        constexpr result<void, container_error> insert(sz idx, auto&&... args) noexcept((sizeof...(args) == 1uz ? INothrowAssignableFrom<decltype(args)...>
                                                                                                                : INothrowConstructibleFrom<T, decltype(args)...>) &&
                                                                                        INothrowMoveConstructible<T> && INothrowMoveAssignable<T> && INothrowDestructible<T>)
        {
            _retif(container_error::out_of_range, idx > this->len);
            if (idx == this->len)
                return this->push(_forward(args)...);
            if (this->len == this->cap)
                _retif(err, container_error err = this->realloc_cap_unless_zero(this->cap > 0_uz ? this->cap * 2_uz : vec::default_initial_capacity(), unsafe);
                       err != container_error::ok);

            if constexpr (ITriviallyMovable<T>)
                std::memmove(this->buf + idx + 1_uz, this->buf + idx, (this->len - idx) * sz(sizeof(T)));
            else
            {
                for (sz i = this->len; i > idx; i--)
                    (*this)[i, unsafe] = std::move((*this)[i - 1_uz, unsafe]);
            }
            if constexpr (sizeof...(args) == 1uz)
                (*this)[idx, unsafe] = _forward(args)...;
            else
                (*this)[idx, unsafe] = T(_forward(args)...);
            ++this->len;

            return {};
        }
        constexpr result<T&, container_error> insert_fetch(sz idx, T val) noexcept;
        constexpr result<T&, container_error> insert_fetch(sz idx, auto&&... args) noexcept;

        constexpr result<void, container_error> append_range(IEnumerable<T> auto&& range) noexcept;

        constexpr result<void, container_error> retain(IFunc<bool(const T&)> auto&& pred) const noexcept;
        constexpr result<void, container_error> retain(IFunc<bool(T&)> auto&& pred) noexcept;

        constexpr result<void, container_error> truncate(sz newLen) noexcept(INothrowDestructible<T>)
        {
            if (newLen >= this->len)
                return result<void, container_error>();

            if constexpr (!ITriviallyDestructible<T>)
            {
                for (T* it = this->buf + (ssz(this->len) - 1_z); it >= this->buf + ssz(newLen); --it)
                {
                    destroy_at(it);
                    if constexpr (!INothrowDestructible<T>)
                        --this->len;
                }
            }
            if constexpr (ITriviallyDestructible<T> || INothrowDestructible<T>)
                this->len = newLen;
        }
        constexpr void force_length(sz newLen, decltype(unsafe)) noexcept
        {
            _contract_assert(newLen <= this->cap);
            this->len = newLen;
        }
        constexpr auto /* result<vec, container_error> */ split_off(sz from) noexcept(INothrowMoveConstructible<T> && INothrowDestructible<T>)
        {
            _retif((result<vec, container_error>(container_error::out_of_range)), from > this->len);

            vec ret;
            ret.assign_from(span(this->buf + from, this->buf + this->len));
            return result<vec, container_error>(std::move(ret));
        }

        constexpr T pop(decltype(unsafe)) noexcept(INothrowMoveConstructible<T> && INothrowDestructible<T>)
        {
            T ret = std::move(this->back(unsafe));
            destroy_at(&this->back(unsafe));
            return ret;
        }
        constexpr result<T> pop() noexcept(INothrowMoveConstructible<T> && INothrowDestructible<T>)
        {
            _retif(nullptr, this->empty());
            return this->pop(unsafe);
        }
        constexpr result<T> swap_and_pop(sz idx, decltype(unsafe)) noexcept(INothrowMoveConstructible<T> && INothrowSwappable<T>)
        {
            using std::swap;

            swap((*this)[idx, unsafe], this->back(unsafe));
            return this->pop(unsafe);
        }
        constexpr result<T> swap_and_pop(sz idx) noexcept(INothrowMoveConstructible<T> && INothrowSwappable<T>)
        {
            _retif(nullptr, idx >= this->size());
            return this->swap_and_pop(idx, unsafe);
        }
        constexpr T remove(sz idx, decltype(unsafe)) noexcept(INothrowMoveAssignable<T> && INothrowDestructible<T>)
        {
            if (idx == this->len - 1_uz)
                return this->pop(unsafe);

            T ret = std::move((*this)[idx, unsafe]);

            if constexpr (ITriviallyMovable<T>)
                std::memmove(this->buf + idx, this->buf + idx + 1_uz, (this->len - idx - 1_uz) * sz(sizeof(T)));
            else
            {
                for (sz i = idx; i < this->len - 1_uz; i++)
                    (*this)[i, unsafe] = std::move((*this)[i + 1_uz, unsafe]);
            }
            destroy_at(&this->back(unsafe));
            --this->len;

            return ret;
        }
        constexpr result<T> remove(sz idx) noexcept(INothrowMoveAssignable<T> && INothrowDestructible<T>)
        {
            _retif(nullptr, idx >= this->size());
            return this->remove(idx, unsafe);
        }
        constexpr void clear() noexcept(INothrowDestructible<T>) { this->truncate(0_uz); }
    };
} // namespace sys
