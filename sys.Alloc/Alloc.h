#pragma once

/// @file

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <type_traits>

#include <AlignedStorage.h>
#include <Integer.h>
#include <LanguageSupport.h>
#include <meta/NamedRequirements.h>
#include <meta/Type.h>

#if !defined(_libcxxext_internal_mock_sup_alloc) || !_libcxxext_internal_mock_sup_alloc
#include <sup/Alloc.h>
#endif

namespace sys::internal
{
    /// @internal
    /// @ingroup sys_internal
    template <typename T>
    consteval sz allocator_sizeof() noexcept
    {
        if constexpr (!std::same_as<T, void>)
            return sz(sizeof(T));
        else
            return 1_uz;
    }
    /// @internal
    /// @ingroup sys_internal
    template <typename T>
    consteval sz allocator_alignof() noexcept
    {
        if constexpr (!std::same_as<T, void>)
            return sz(alignof(T));
        else
            return sz(alignof(std::max_align_t));
    }
    /// @internal
    /// @ingroup sys_internal
    template <typename T>
    consteval sz max_alloc_elem_count(const sz maxSizeBytes = sz::highest()) noexcept
    {
        return maxSizeBytes / internal::allocator_sizeof<T>();
    }
} // namespace sys::internal

namespace sys
{
    /// @ingroup sys_alloc
    /// @brief A generic allocator type.
    /// @details
    /// Conformant library allocators satisfy `sys::IAllocator` and implement the following semantics.
    /// -   Alignments must be provided as powers of two, which also implies `>= 1`.
    /// -   Memory blocks must suffice an alignment of a. `alignof(T)`, when `T` is non-`void`, or b. `alignof(std::max_align_t)`, when `T` is `void`.
    /// -   `.alloc(...)` and `.alloc_zeroed(...)` produce a non-`nullptr` `T*` sufficiently aligned for a. `T`, when non-`void`, or b. `std::max_align_t`, when `T` is `void`.
    /// -   `.alloc_zeroed(...)`, on success, produces a pointer to a block which is zero-initialized, i.e. the inspection of each bit within the contiguous range must be
    /// observably zero.
    /// -   `.will_realloc_inplace(...)` returns `true` iff. a subsequent call to `.realloc(...)` with _the exact same parameters_, is guaranteed to not move the memory block
    /// referred to by the pointer argument, _shall such a reallocation succeed_.
    /// -   `.realloc(...)` with a non-`nullptr` pointer argument produces a pointer to a block which is prefixed by a range bit-equal to that of the range of the passed-in block,
    /// up to the ends of both blocks, whichever is lesser. The old block must be described exactly by the element count and alignment arguments, and the new block shall be, on
    /// success, described exactly by the new element count and alignment arguments. If the reallocation fails, the old block shall remain valid and unchanged.
    /// -   `.realloc(...)` with a `nullptr` pointer argument behaves identically to `.alloc(...)`; the first element count and alignment arguments shall be ignored.
    /// -   `.dealloc(...)` with a non-`nullptr` pointer argument deallocates the block of memory referred to by the pointer argument, which _must_ have been obtained by the same
    /// allocator object, and described exactly by the element count and alignment arguments.
    /// -   `.dealloc(...)` with a `nullptr` pointer argument shall be a no-op; the element count and alignment arguments shall be ignored.
    /// -   An exception to block-owning allocators is if `.is_stateless(ptr)` is `true`, which gives permission for `ptr` to be ping-ponged to another instance of an allocator
    /// object for lifetime management. Note that pointers to blocks must still be obtained from _some_ allocator object of the same type, and deallocation of non-`nullptr`
    /// pointers must still be performed with the correct element count and alignment arguments.
    ///
    /// Lifetime assumptions!
    /// @code{.cpp}
    /// IAllocator<Type> auto allocator /* = ... */;
    /// ...
    /// Type* amem = allocator.alloc(elementCount, (align), unsafe);
    /// Type* azmem = allocator.alloc_zeroed(elementCount, (align), unsafe);
    /// Type* rmem = allocator.realloc(... /* or `nullptr` */, elementCount, (align), unsafe);
    /// ...
    /// allocator.dealloc(nullptr, elementCount, (align), unsafe); // Must be ok!
    /// allocator.dealloc(rmem, elementCount, (align), unsafe);
    /// allocator.dealloc(azmem, elementCount, (align), unsafe);
    /// allocator.dealloc(amem, elementCount, (align), unsafe);
    /// ...
    /// allocator.~decltype(allocator)();
    /// @endcode
    template <typename Allocator, typename T = void>
    concept IAllocator = meta::type<T>::is_unqualified() && meta::type<Allocator>::is_unqualified() && INothrowDefaultConstructible<Allocator> && INothrowDestructible<Allocator> &&
        requires(Allocator allocator, sz elementCount, sz align, sz newElementCount, sz newAlign) {
            { _as(allocator, const Allocator&).is_stateless(_as(nullptr, const T*)) } -> IBooleanTestable;
            { allocator.alloc(elementCount, unsafe) } -> std::same_as<T*>;
            { allocator.alloc(elementCount, align, unsafe) } -> std::same_as<T*>;
            { allocator.alloc_zeroed(elementCount, unsafe) } -> std::same_as<T*>;
            { allocator.alloc_zeroed(elementCount, align, unsafe) } -> std::same_as<T*>;
            { allocator.will_realloc_inplace(_as(nullptr, T*), elementCount, newElementCount, unsafe) } -> IBooleanTestable;
            { allocator.will_realloc_inplace(_as(nullptr, T*), elementCount, align, newElementCount, newAlign, unsafe) } -> IBooleanTestable;
            { allocator.realloc(_as(nullptr, T*), elementCount, newElementCount, unsafe) } -> std::same_as<T*>;
            { allocator.realloc(_as(nullptr, T*), elementCount, align, newElementCount, newAlign, unsafe) } -> std::same_as<T*>;
            { allocator.dealloc(_as(nullptr, T*), elementCount, unsafe) } -> std::same_as<void>;
            { allocator.dealloc(_as(nullptr, T*), elementCount, align, unsafe) } -> std::same_as<void>;
        };

    /// @ingroup sys_alloc
    /// @brief A default allocator, based on the standard C heap management functions.
    /// @details Implements `sys::INothrowDefaultConstructible`, `sys::INothrowDestructible`.
    template <typename T = void>
    requires (meta::type<T>::is_unqualified())
    class system_allocator /* Not `final` to enable deriving allocators. */
    {
    public:
        constexpr system_allocator() noexcept = default;
        constexpr system_allocator(const system_allocator&) noexcept = default;
        constexpr system_allocator(system_allocator&&) noexcept = default;
        constexpr ~system_allocator() noexcept = default;

        constexpr system_allocator& operator=(const system_allocator&) noexcept = default;
        constexpr system_allocator& operator=(system_allocator&&) noexcept = default;

        /// .
        static constexpr bool is_stateless(const T*) noexcept { return true; }

        /// .
        [[nodiscard]] constexpr T* alloc(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, elementCount > internal::max_alloc_elem_count<T>() || align > alignof(std::max_align_t)); // LCOV_EXCL_BR_LINE
            return _as(internal::global_alloc(elementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
        }
        /// .
        [[nodiscard]] constexpr T* alloc(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc(elementCount, internal::allocator_alignof<T>(), unsafe); }

        /// .
        [[nodiscard]] constexpr T* alloc_zeroed(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, elementCount > internal::max_alloc_elem_count<T>() || align > alignof(std::max_align_t)); // LCOV_EXCL_BR_LINE
            return _as(internal::global_alloc_zeroed(elementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
        }
        /// .
        [[nodiscard]] constexpr T* alloc_zeroed(const sz elementCount, decltype(unsafe)) noexcept
        {
            return this->alloc_zeroed(elementCount, internal::allocator_alignof<T>(), unsafe);
        }

        /// .
        [[nodiscard]] static constexpr bool will_realloc_inplace([[maybe_unused]] T* ptr, [[maybe_unused]] const sz elementCount, [[maybe_unused]] const sz align,
                                                                 [[maybe_unused]] const sz newElementCount, [[maybe_unused]] const sz newAlign, decltype(unsafe)) noexcept
        {
            return false;
        }
        /// .
        [[nodiscard]] static constexpr bool will_realloc_inplace([[maybe_unused]] T* ptr, [[maybe_unused]] const sz elementCount, [[maybe_unused]] const sz newElementCount,
                                                                 decltype(unsafe)) noexcept
        {
            return false;
        }

        /// .
        [[nodiscard]] constexpr T* realloc(T* ptr, const sz elementCount, const sz align, const sz newElementCount, const sz newAlign, decltype(unsafe)) noexcept
        {
            _retif(nullptr, newElementCount > internal::max_alloc_elem_count<T>() || newAlign > alignof(std::max_align_t)); // LCOV_EXCL_BR_LINE
            return _as(internal::global_realloc(ptr, elementCount * internal::allocator_sizeof<T>(), align, newElementCount * internal::allocator_sizeof<T>(), newAlign, unsafe),
                       T*);
        }
        /// .
        [[nodiscard]] constexpr T* realloc(T* ptr, const sz elementCount, const sz newElementCount, decltype(unsafe)) noexcept
        {
            return this->realloc(ptr, elementCount, internal::allocator_alignof<T>(), newElementCount, internal::allocator_alignof<T>(), unsafe);
        }

        /// .
        constexpr void dealloc(T* ptr, const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            internal::global_dealloc(ptr, elementCount * internal::allocator_sizeof<T>(), align, unsafe);
        }
        /// .
        constexpr void dealloc(T* ptr, const sz elementCount, decltype(unsafe)) noexcept { this->dealloc(ptr, elementCount, internal::allocator_alignof<T>(), unsafe); }
    };

    /// @ingroup sys_alloc
    /// @brief A single-block allocator that uses a fixed-size buffer to allocate an exclusive block of memory.
    /// @details Implements `sys::INothrowDefaultConstructible`, `sys::INothrowDestructible`.
    template <typename T = void,
              size_t BufferElementCount = std::min(32uz /* NOLINT(readability-magic-numbers) */ / *internal::allocator_sizeof<T>(), 16uz /* NOLINT(readability-magic-numbers) */)>
    requires requires {
        requires (meta::type<T>::is_unqualified());
        requires BufferElementCount <= internal::max_alloc_elem_count<T>();
        requires BufferElementCount > 0uz;
    }
    class inplace_allocator /* Not `final` to enable deriving allocators. */
    {
        using value_type = std::conditional_t<!std::same_as<T, void>, T, byte>;

        alignas(*internal::allocator_alignof<T>()) aligned_storage<value_type[BufferElementCount]> storage;
        bool used = false;
    public:
        constexpr inplace_allocator() noexcept = default;
        constexpr inplace_allocator(const inplace_allocator&) noexcept = delete;
        constexpr inplace_allocator(inplace_allocator&&) noexcept = delete;
        constexpr ~inplace_allocator() noexcept = default;

        constexpr inplace_allocator& operator=(const inplace_allocator&) noexcept = delete;
        constexpr inplace_allocator& operator=(inplace_allocator&&) noexcept = delete;

        /// .
        static constexpr bool is_stateless(const T*) noexcept { return false; }

        /// .
        [[nodiscard]] constexpr T* alloc(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, this->used || elementCount > BufferElementCount || align > internal::allocator_alignof<T>());
            this->used = true;
            return *this->storage.data();
        }
        /// .
        [[nodiscard]] constexpr T* alloc(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc(elementCount, internal::allocator_alignof<T>(), unsafe); }

        /// .
        [[nodiscard]] constexpr T* alloc_zeroed(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, this->used || elementCount > BufferElementCount || align > internal::allocator_alignof<T>()); // LCOV_EXCL_BR_LINE
            std::memset(this->storage.data(), 0, elementCount * internal::allocator_sizeof<T>());
            this->used = true;
            return *this->storage.data();
        }
        /// .
        [[nodiscard]] constexpr T* alloc_zeroed(const sz elementCount, decltype(unsafe)) noexcept
        {
            return this->alloc_zeroed(elementCount, internal::allocator_alignof<T>(), unsafe);
        }

        /// .
        [[nodiscard]] static constexpr bool will_realloc_inplace([[maybe_unused]] T* ptr, [[maybe_unused]] const sz elementCount, [[maybe_unused]] const sz align,
                                                                 [[maybe_unused]] const sz newElementCount, [[maybe_unused]] const sz newAlign, decltype(unsafe)) noexcept
        {
            return true;
        }
        /// .
        [[nodiscard]] static constexpr bool will_realloc_inplace([[maybe_unused]] T* ptr, [[maybe_unused]] const sz elementCount, [[maybe_unused]] const sz newElementCount,
                                                                 decltype(unsafe)) noexcept
        {
            return true;
        }

        /// .
        [[nodiscard]] constexpr T* realloc(T* ptr, [[maybe_unused]] const sz elementCount, [[maybe_unused]] const sz align, const sz newElementCount, const sz newAlign,
                                           decltype(unsafe)) noexcept
        {
            _retif(nullptr, (this->used && *this->storage.data() != ptr) || newElementCount > BufferElementCount || newAlign > internal::allocator_alignof<T>());
            this->used = true; // Because `ptr` can be `nullptr` and thus initially `this->used == false`.
            return *this->storage.data();
        }
        /// .
        [[nodiscard]] constexpr T* realloc(T* ptr, const sz elementCount, const sz newElementCount, decltype(unsafe)) noexcept
        {
            return this->realloc(ptr, elementCount, internal::allocator_alignof<T>(), newElementCount, internal::allocator_alignof<T>(), unsafe);
        }

        /// .
        constexpr void dealloc(T* ptr, const sz, const sz, decltype(unsafe)) noexcept
        {
            _retif(, !ptr);
            this->used = false;
        }
        /// .
        constexpr void dealloc(T* ptr, const sz elementCount, decltype(unsafe)) noexcept { this->dealloc(ptr, elementCount, internal::allocator_alignof<T>(), unsafe); }
    };

    /// @ingroup sys_alloc
    /// @brief A hybrid allocator that will always try to use small buffer optimization via a single internal buffer for small allocations, and falls back to the system allocator
    /// otherwise.
    /// @details Implements `sys::INothrowDefaultConstructible`, `sys::INothrowDestructible`.
    template <typename T = void,
              size_t BufferElementCount = std::min(32uz /* NOLINT(readability-magic-numbers) */ / *internal::allocator_sizeof<T>(), 16uz /* NOLINT(readability-magic-numbers) */)>
    requires requires {
        requires (meta::type<T>::is_unqualified());
        requires BufferElementCount <= internal::max_alloc_elem_count<T>();
        requires BufferElementCount > 0uz;
    }
    class small_buffer_allocator /* Not `final` to enable deriving allocators. */
    {
        using value_type = std::conditional_t<!std::same_as<T, void>, T, byte>;

        alignas(*internal::allocator_alignof<T>()) aligned_storage<value_type[BufferElementCount]> storage;
        bool storage_used = false;
    public:
        constexpr small_buffer_allocator() noexcept = default;
        constexpr small_buffer_allocator(const small_buffer_allocator&) noexcept = delete;
        constexpr small_buffer_allocator(small_buffer_allocator&&) noexcept = delete;
        constexpr ~small_buffer_allocator() noexcept = default;

        constexpr small_buffer_allocator& operator=(const small_buffer_allocator&) noexcept = delete;
        constexpr small_buffer_allocator& operator=(small_buffer_allocator&&) noexcept = delete;

        /// .
        bool is_stateless(const T* ptr) const noexcept { return *this->storage.data() != ptr; }

        /// .
        [[nodiscard]] constexpr T* alloc(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, elementCount > internal::max_alloc_elem_count<T>() || align > internal::allocator_alignof<T>());

            if (!this->storage_used && elementCount <= BufferElementCount)
            {
                this->storage_used = true;
                return *this->storage.data();
            }

            return _as(internal::global_alloc(elementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
        }
        /// .
        [[nodiscard]] constexpr T* alloc(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc(elementCount, internal::allocator_alignof<T>(), unsafe); }

        /// .
        [[nodiscard]] constexpr T* alloc_zeroed(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, elementCount > internal::max_alloc_elem_count<T>() || align > internal::allocator_alignof<T>());

            if (!this->storage_used && elementCount <= BufferElementCount)
            {
                this->storage_used = true;
                std::memset(this->storage.data(), 0, elementCount * internal::allocator_sizeof<T>());
                return *this->storage.data();
            }

            return _as(internal::global_alloc_zeroed(elementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
        }
        /// .
        [[nodiscard]] constexpr T* alloc_zeroed(const sz elementCount, decltype(unsafe)) noexcept
        {
            return this->alloc_zeroed(elementCount, internal::allocator_alignof<T>(), unsafe);
        }

        /// .
        [[nodiscard]] bool will_realloc_inplace(T* ptr, const sz elementCount, [[maybe_unused]] const sz align, const sz newElementCount, [[maybe_unused]] const sz newAlign,
                                                decltype(unsafe)) noexcept
        {
            return *this->storage.data() == ptr && elementCount <= BufferElementCount && newElementCount <= BufferElementCount;
        }
        /// .
        [[nodiscard]] bool will_realloc_inplace(T* ptr, const sz elementCount, const sz newElementCount, decltype(unsafe)) noexcept
        {
            return this->will_realloc_inplace(ptr, elementCount, internal::allocator_alignof<T>(), newElementCount, internal::allocator_alignof<T>(), unsafe);
        }

        /// .
        [[nodiscard]] constexpr T* realloc(T* ptr, const sz elementCount, const sz align, const sz newElementCount, const sz newAlign, decltype(unsafe)) noexcept
        {
            _retif(nullptr, newElementCount > internal::max_alloc_elem_count<T>() || newAlign > internal::allocator_alignof<T>());

            if ((*this->storage.data() == ptr || !this->storage_used) && newElementCount <= BufferElementCount)
            {
                if (ptr && *this->storage.data() != ptr)
                {
                    _nowarn_begin_one_gcc("-Wclass-memaccess");
                    std::memcpy(*this->storage.data(), ptr, newElementCount * internal::allocator_sizeof<T>());
                    _nowarn_end_gcc();
                    internal::global_dealloc(ptr, elementCount, align, unsafe);
                }

                this->storage_used = true;
                return *this->storage.data();
            }

            if (*this->storage.data() == ptr && newElementCount > BufferElementCount)
            {
                T* ret = _as(internal::global_alloc(newElementCount * internal::allocator_sizeof<T>(), newAlign, unsafe), T*);
                _retif(nullptr, !ret);

                this->storage_used = false;
                _nowarn_begin_one_gcc("-Wclass-memaccess");
                std::memcpy(ret, *this->storage.data(), elementCount * internal::allocator_sizeof<T>());
                _nowarn_end_gcc();
                return ret;
            }

            return _as(internal::global_realloc(ptr, elementCount * internal::allocator_sizeof<T>(), align, newElementCount * internal::allocator_sizeof<T>(), newAlign, unsafe),
                       T*);
        }
        /// .
        [[nodiscard]] constexpr T* realloc(T* ptr, const sz elementCount, const sz newElementCount, decltype(unsafe)) noexcept
        {
            return this->realloc(ptr, elementCount, internal::allocator_alignof<T>(), newElementCount, internal::allocator_alignof<T>(), unsafe);
        }

        /// .
        constexpr void dealloc(T* ptr, const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            if (*this->storage.data() == ptr)
            {
                this->storage_used = false;
                return;
            }

            internal::global_dealloc(ptr, elementCount * internal::allocator_sizeof<T>(), align, unsafe);
        }
        /// .
        constexpr void dealloc(T* ptr, const sz elementCount, decltype(unsafe)) noexcept { this->dealloc(ptr, elementCount, internal::allocator_alignof<T>(), unsafe); }
    };
} // namespace sys
