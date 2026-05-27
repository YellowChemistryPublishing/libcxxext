#pragma once

/// @file

#include <cstring>

#include <LanguageSupport.h>
#include <meta/NamedRequirements.h>
#include <meta/Type.h>

namespace sys
{
    /// .
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    _inline_always constexpr void construct_at(T* dest, const T& with)
    {
        if constexpr (ITriviallyCopyable<T>)
            std::memcpy(dest, &with, sizeof(T));
        else
            std::construct_at(dest, with);
    }
    /// .
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    _inline_always constexpr void construct_at(T* dest, T&& with)
    {
        if constexpr (ITriviallyMoveable<T>)
            std::memcpy(dest, &with, sizeof(T));
        else
            std::construct_at(dest, with);
    }
    /// .
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    _inline_always constexpr void construct_at(T* dest, auto&&... args)
    {
        std::construct_at(dest, _forward(args)...);
    }

    /// .
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    _inline_always constexpr void destroy_at(T* obj) noexcept
    {
        if constexpr (!ITriviallyDestructible<T>)
            std::destroy_at(obj);
    }
} // namespace sys
