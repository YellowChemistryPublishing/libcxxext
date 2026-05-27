#pragma once

/// @file

#include <LanguageSupport.h>
#include <meta/Type.h>

namespace sys::traits
{
    /// @ingroup sys
    /// @brief A tag type for contiguous ranges.
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    struct contiguous_range
    { };

    /// @ingroup sys
    /// @brief A tag type for random access ranges.
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    struct random_access_range
    { };

    /// @ingroup sys
    /// @brief A tag type for bidirectional ranges.
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    struct bidirectional_range
    { };

    /// @ingroup sys
    /// @brief A tag type for forward ranges.
    template <typename T>
    requires (meta::type<T>::is_unqualified())
    struct forward_range
    { };

    /// @ingroup sys
    /// @brief Trait for a contiguous iterator.
    template <typename T, typename ValueType>
    requires (meta::type<T>::is_unqualified())
    struct contiguous_iterator
    {
        using iterator_category = std::contiguous_iterator_tag;

        using value_type = ValueType;
        using difference_type = ssz;

        using pointer = ValueType*;
        using reference = ValueType&;
    };

    /// @ingroup sys
    /// @brief Trait for a random access iterator.
    template <typename T, typename ValueType>
    requires (meta::type<T>::is_unqualified())
    struct random_access_iterator
    {
        using iterator_category = std::random_access_iterator_tag;

        using value_type = ValueType;
        using difference_type = ssz;

        using pointer = ValueType*;
        using reference = ValueType&;
    };

    /// @ingroup sys
    /// @brief Trait for a bidirectional iterator.
    template <typename T, typename ValueType>
    requires (meta::type<T>::is_unqualified())
    struct bidirectional_iterator
    {
        using iterator_category = std::bidirectional_iterator_tag;

        using value_type = ValueType;
        using difference_type = ssz;

        using pointer = ValueType*;
        using reference = ValueType&;
    };

    /// @ingroup sys
    /// @brief Trait for a forward iterator.
    template <typename T, typename ValueType>
    requires (meta::type<T>::is_unqualified())
    struct forward_iterator
    {
        using iterator_category = std::forward_iterator_tag;

        using value_type = ValueType;
        using difference_type = ssz;

        using pointer = ValueType*;
        using reference = ValueType&;
    };
} // namespace sys::traits
