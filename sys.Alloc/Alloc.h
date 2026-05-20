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
    template <typename T, typename Type = void>
    concept IAllocator =
        meta::type<T>::is_unqualified() && std::same_as<Type, std::remove_reference_t<Type>> && requires(T allocator, sz elementCount, sz align, sz newElementCount, sz newAlign) {
            { allocator.alloc(elementCount, unsafe) } -> std::same_as<Type*>;
            { allocator.alloc(elementCount, align, unsafe) } -> std::same_as<Type*>;
            { allocator.alloc_zeroed(elementCount, unsafe) } -> std::same_as<Type*>;
            { allocator.alloc_zeroed(elementCount, align, unsafe) } -> std::same_as<Type*>;
            { allocator.realloc(_as(nullptr, Type*), elementCount, newElementCount, unsafe) } -> std::same_as<Type*>;
            { allocator.realloc(_as(nullptr, Type*), elementCount, align, newElementCount, newAlign, unsafe) } -> std::same_as<Type*>;
            { allocator.dealloc(_as(nullptr, Type*), elementCount, unsafe) } -> std::same_as<void>;
            { allocator.dealloc(_as(nullptr, Type*), elementCount, align, unsafe) } -> std::same_as<void>;
        };

    /// @ingroup sys_alloc
    /// @brief A default allocator, based on the standard C heap management functions.
    /// @details Implements `sys::INothrowDefaultConstructible`, `sys::INothrowDestructible`.
    template <typename T = void>
    requires (meta::type<T>::is_unqualified())
    class system_allocator /* Not `final` to enable deriving allocators. */
    {
    public:
        system_allocator() noexcept = default;
        system_allocator(const system_allocator&) noexcept = delete;
        system_allocator(system_allocator&&) noexcept = delete;
        ~system_allocator() noexcept = default;

        system_allocator& operator=(const system_allocator&) noexcept = delete;
        system_allocator& operator=(system_allocator&&) noexcept = delete;

        /// .
        [[nodiscard]] T* alloc(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, elementCount > internal::max_alloc_elem_count<T>() || align > alignof(std::max_align_t)); // LCOV_EXCL_BR_LINE
            return _as(internal::global_alloc(elementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
        }
        /// .
        [[nodiscard]] T* alloc(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc(elementCount, internal::allocator_alignof<T>(), unsafe); }
        /// .
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, elementCount > internal::max_alloc_elem_count<T>() || align > alignof(std::max_align_t)); // LCOV_EXCL_BR_LINE
            return _as(internal::global_alloc_zeroed(elementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
        }
        /// .
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc_zeroed(elementCount, internal::allocator_alignof<T>(), unsafe); }
        /// .
        [[nodiscard]] T* realloc(T* ptr, const sz elementCount, const sz align, const sz newElementCount, const sz newAlign, decltype(unsafe)) noexcept
        {
            _retif(nullptr, newElementCount > internal::max_alloc_elem_count<T>() || newAlign > alignof(std::max_align_t)); // LCOV_EXCL_BR_LINE
            return _as(internal::global_realloc(ptr, elementCount * internal::allocator_sizeof<T>(), align, newElementCount * internal::allocator_sizeof<T>(), newAlign, unsafe),
                       T*);
        }
        /// .
        [[nodiscard]] T* realloc(T* ptr, const sz elementCount, const sz newElementCount, decltype(unsafe)) noexcept
        {
            return this->realloc(ptr, elementCount, internal::allocator_alignof<T>(), newElementCount, internal::allocator_alignof<T>(), unsafe);
        }
        /// .
        void dealloc(T* ptr, const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            internal::global_dealloc(ptr, elementCount * internal::allocator_sizeof<T>(), align, unsafe);
        }
        /// .
        void dealloc(T* ptr, const sz elementCount, decltype(unsafe)) noexcept { this->dealloc(ptr, elementCount, internal::allocator_alignof<T>(), unsafe); }
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
    class small_buffer_allocator /* Not `final` to enable deriving allocators. */
    {
        using value_type = std::conditional_t<!std::same_as<T, void>, T, byte>;

        alignas(*internal::allocator_alignof<T>()) aligned_storage<value_type[BufferElementCount]> storage;
        bool used = false;
    public:
        small_buffer_allocator() noexcept = default;
        small_buffer_allocator(const small_buffer_allocator&) noexcept = delete;
        small_buffer_allocator(small_buffer_allocator&&) noexcept = delete;
        ~small_buffer_allocator() noexcept = default;

        small_buffer_allocator& operator=(const small_buffer_allocator&) noexcept = delete;
        small_buffer_allocator& operator=(small_buffer_allocator&&) noexcept = delete;

        /// .
        [[nodiscard]] T* alloc(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, this->used || elementCount > BufferElementCount || align > internal::allocator_alignof<T>());
            this->used = true;
            return *this->storage.data();
        }
        /// .
        [[nodiscard]] T* alloc(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc(elementCount, internal::allocator_alignof<T>(), unsafe); }
        /// .
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, this->used || elementCount > BufferElementCount || align > internal::allocator_alignof<T>()); // LCOV_EXCL_BR_LINE
            (void)std::memset(this->storage.data(), 0, elementCount * internal::allocator_sizeof<T>());
            this->used = true;
            return *this->storage.data();
        }
        /// .
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc_zeroed(elementCount, internal::allocator_alignof<T>(), unsafe); }
        /// .
        [[nodiscard]] T* realloc(T* ptr, [[maybe_unused]] const sz elementCount, [[maybe_unused]] const sz align, const sz newElementCount, const sz newAlign,
                                 decltype(unsafe)) noexcept
        {
            _retif(nullptr, (this->used && *this->storage.data() != ptr) || newElementCount > BufferElementCount || newAlign > internal::allocator_alignof<T>());
            this->used = true;
            return *this->storage.data();
        }
        /// .
        [[nodiscard]] T* realloc(T* ptr, const sz elementCount, const sz newElementCount, decltype(unsafe)) noexcept
        {
            return this->realloc(ptr, elementCount, internal::allocator_alignof<T>(), newElementCount, internal::allocator_alignof<T>(), unsafe);
        }
        /// .
        void dealloc(T* ptr, const sz, const sz, decltype(unsafe)) noexcept
        {
            _retif(, !ptr);
            this->used = false;
        }
        /// .
        void dealloc(T* ptr, const sz elementCount, decltype(unsafe)) noexcept { this->dealloc(ptr, elementCount, internal::allocator_alignof<T>(), unsafe); }
    };

    /// @ingroup sys_alloc
    /// @brief A single-block, hybrid allocator that uses small buffer optimization for small allocations, and falls back to the system allocator for large allocations.
    /// @details Implements `sys::INothrowDefaultConstructible`, `sys::INothrowDestructible`.
    template <typename T = void,
              size_t BufferElementCount = std::min(32uz /* NOLINT(readability-magic-numbers) */ / *internal::allocator_sizeof<T>(), 16uz /* NOLINT(readability-magic-numbers) */)>
    requires requires {
        requires (meta::type<T>::is_unqualified());
        requires BufferElementCount <= internal::max_alloc_elem_count<T>();
        requires BufferElementCount > 0uz;
    }
    class dynamic_allocator /* Not `final` to enable deriving allocators. */
    {
        using value_type = std::conditional_t<!std::same_as<T, void>, T, byte>;

        alignas(*internal::allocator_alignof<T>()) aligned_storage<value_type[BufferElementCount]> storage;
        enum class mem_type : byte
        {
            empty,
            small_buffer,
            system,
        } alloc_type = mem_type::empty;
    public:
        dynamic_allocator() noexcept = default;
        dynamic_allocator(const dynamic_allocator&) noexcept = delete;
        dynamic_allocator(dynamic_allocator&&) noexcept = delete;
        ~dynamic_allocator() noexcept = default;

        dynamic_allocator& operator=(const dynamic_allocator&) noexcept = delete;
        dynamic_allocator& operator=(dynamic_allocator&&) noexcept = delete;

        /// .
        [[nodiscard]] T* alloc(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, this->alloc_type != mem_type::empty || elementCount > internal::max_alloc_elem_count<T>() || align > internal::allocator_alignof<T>());

            if (elementCount <= BufferElementCount)
            {
                this->alloc_type = mem_type::small_buffer;
                return *this->storage.data();
            }

            T* ret = _as(internal::global_alloc(elementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
            _retif(nullptr, !ret);

            this->alloc_type = mem_type::system;
            return ret;
        }
        /// .
        [[nodiscard]] T* alloc(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc(elementCount, internal::allocator_alignof<T>(), unsafe); }
        /// .
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            /* LCOV_EXCL_BR_LINE */ _retif(nullptr,
                                           this->alloc_type != mem_type::empty || elementCount > internal::max_alloc_elem_count<T>() || align > internal::allocator_alignof<T>());

            if (elementCount <= BufferElementCount)
            {
                (void)std::memset(this->storage.data(), 0, elementCount * internal::allocator_sizeof<T>());
                this->alloc_type = mem_type::small_buffer;
                return *this->storage.data();
            }

            T* ret = _as(internal::global_alloc_zeroed(elementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
            _retif(nullptr, !ret);

            this->alloc_type = mem_type::system;
            return ret;
        }
        /// .
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc_zeroed(elementCount, internal::allocator_alignof<T>(), unsafe); }
        /// .
        [[nodiscard]] T* realloc(T* ptr, const sz elementCount, const sz align, const sz newElementCount, const sz newAlign, decltype(unsafe)) noexcept
        {
            _retif(nullptr, newElementCount > internal::max_alloc_elem_count<T>() || newAlign > internal::allocator_alignof<T>());

            if (newElementCount <= BufferElementCount)
            {
                if (this->alloc_type == mem_type::system)
                {
                    (void)std::memcpy(*this->storage.data(), ptr, newElementCount * internal::allocator_sizeof<T>());
                    internal::global_dealloc(ptr, elementCount, align, unsafe);
                    this->alloc_type = mem_type::small_buffer;
                }

                return *this->storage.data();
            }

            switch (this->alloc_type) // LCOV_EXCL_BR_LINE
            {
            case mem_type::small_buffer:
                {
                    T* ret = _as(internal::global_alloc(newElementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
                    _retif(nullptr, !ret);

                    (void)std::memcpy(ret, *this->storage.data(), BufferElementCount * internal::allocator_sizeof<T>());
                    this->alloc_type = mem_type::system;
                    return ret;
                }
            case mem_type::system:
                return _as(
                    internal::global_realloc(ptr, elementCount * internal::allocator_sizeof<T>(), align, newElementCount * internal::allocator_sizeof<T>(), newAlign, unsafe), T*);
            case mem_type::empty:
                {
                    T* ret = _as(internal::global_alloc(newElementCount * internal::allocator_sizeof<T>(), align, unsafe), T*);
                    _retif(nullptr, !ret);

                    this->alloc_type = mem_type::system;
                    return ret;
                }
            default: return nullptr; // LCOV_EXCL_LINE
            }
        }
        /// .
        [[nodiscard]] T* realloc(T* ptr, const sz elementCount, const sz newElementCount, decltype(unsafe)) noexcept
        {
            return this->realloc(ptr, elementCount, internal::allocator_alignof<T>(), newElementCount, internal::allocator_alignof<T>(), unsafe);
        }
        /// .
        void dealloc(T* ptr, const sz, const sz, decltype(unsafe)) noexcept
        {
            if (this->alloc_type == mem_type::system)
                internal::global_dealloc(ptr, 0_uz, 0_uz, unsafe);

            this->alloc_type = mem_type::empty;
        }
        /// .
        void dealloc(T* ptr, const sz elementCount, decltype(unsafe)) noexcept { this->dealloc(ptr, elementCount, internal::allocator_alignof<T>(), unsafe); }
    };
} // namespace sys
