#pragma once

/// @file

#include <algorithm>
#include <cstddef>
#include <type_traits>

#include <AlignedStorage.h>
#include <LanguageSupport.h>
#include <sup/Alloc.h>

namespace sys::internal
{
    /// @internal
    /// @ingroup sys_internal
    template <typename T>
    consteval sz allocator_element_size() noexcept
    {
        if constexpr (!std::same_as<T, void>)
            return sz(sizeof(T));
        else
            return 1_uz;
    }
    /// @internal
    /// @ingroup sys_internal
    template <typename T>
    consteval sz allocator_align() noexcept
    {
        if constexpr (!std::same_as<T, void>)
            return sz(alignof(T));
        else
            return sz(alignof(std::max_align_t));
    }
} // namespace sys::internal

namespace sys
{
    /// @ingroup sys_alloc
    /// @brief A default allocator, based on the standard C heap management functions.
    template <typename T = void>
    requires std::same_as<T, std::remove_cvref_t<T>>
    class system_allocator /* Not `final` to enable deriving allocators. */
    {
    public:
        [[nodiscard]] T* alloc(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            return _as(internal::alloc(elementCount * internal::allocator_element_size<T>(), align, unsafe), T*);
        }
        [[nodiscard]] T* alloc(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc(elementCount, internal::allocator_align<T>(), unsafe); }
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            return _as(internal::alloc_zeroed(elementCount * internal::allocator_element_size<T>(), align, unsafe), T*);
        }
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc_zeroed(elementCount, internal::allocator_align<T>(), unsafe); }
        [[nodiscard]] T* realloc(T* ptr, const sz newElementCount, const sz align, decltype(unsafe)) noexcept
        {
            return _as(internal::realloc(ptr, newElementCount * internal::allocator_element_size<T>(), align, unsafe), T*);
        }
        [[nodiscard]] T* realloc(T* ptr, const sz newElementCount, decltype(unsafe)) noexcept
        {
            return this->realloc(ptr, newElementCount, internal::allocator_align<T>(), unsafe);
        }
        void dealloc(T* ptr, const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            internal::dealloc(ptr, elementCount * internal::allocator_element_size<T>(), align, unsafe);
        }
        void dealloc(T* ptr, const sz elementCount, decltype(unsafe)) noexcept { this->dealloc(ptr, elementCount, internal::allocator_align<T>(), unsafe); }
    };

    /// @ingroup sys_alloc
    /// @brief A default allocator, based on the standard C heap management functions.
    template <typename T = void, size_t BufferElementCount = std::min(32uz / sizeof(T), 16uz)>
    requires std::same_as<T, std::remove_cvref_t<T>>
    class small_buffer_allocator /* Not `final` to enable deriving allocators. */
    {
        using value_type = std::conditional_t<!std::same_as<T, void>, T, byte>;

        aligned_storage<value_type[BufferElementCount]> storage;
        bool used = false;
    public:
        [[nodiscard]] T* alloc(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            //                                                                  v Argh. Hope no one tries a silly number for `align`.
            _retif(nullptr, this->used || elementCount > BufferElementCount || align < internal::allocator_align<T>());
            this->used = true;
            return _as(_as(this->storage.data(), void*), T*);
        }
        [[nodiscard]] T* alloc(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc(elementCount, internal::allocator_align<T>(), unsafe); }
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, this->used || elementCount > BufferElementCount || align < internal::allocator_align<T>()); // LCOV_EXCL_BR_LINE
            (void)std::memset(this->storage.data(), 0, elementCount * internal::allocator_element_size<T>());
            this->used = true;
            return _as(_as(this->storage.data(), void*), T*);
        }
        [[nodiscard]] T* alloc_zeroed(const sz elementCount, decltype(unsafe)) noexcept { return this->alloc_zeroed(elementCount, internal::allocator_align<T>(), unsafe); }
        [[nodiscard]] T* realloc(T* ptr, const sz newElementCount, const sz align, decltype(unsafe)) noexcept
        {
            _retif(nullptr, (this->used && !ptr) || newElementCount > BufferElementCount || align < internal::allocator_align<T>());
            this->used = true;
            return _as(_as(this->storage.data(), void*), T*);
        }
        [[nodiscard]] T* realloc(T* ptr, const sz newElementCount, decltype(unsafe)) noexcept
        {
            return this->realloc(ptr, newElementCount, internal::allocator_align<T>(), unsafe);
        }
        void dealloc(T* ptr, const sz, const sz, decltype(unsafe)) noexcept
        {
            _retif(, !ptr);
            this->used = false;
        }
        void dealloc(T* ptr, const sz elementCount, decltype(unsafe)) noexcept { this->dealloc(ptr, elementCount, internal::allocator_align<T>(), unsafe); }
    };
} // namespace sys
