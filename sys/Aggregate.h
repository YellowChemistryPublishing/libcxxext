#pragma once

/// @file

#include <stdexcept>
#include <utility>

#include <Integer.h>
#include <meta/Type.h>

namespace sys
{
    // TODO(halloimdragon): Pattern matching mechanism?

    /// @brief A simple aggregate type that holds a fixed set of heterogeneous values.
    template <typename... Ts>
    struct aggregate final
    {
        static_assert(false,
                      "To support memory layout guarantees, aggregate initialization requirements, and structured binding support, specializations for `aggregate` are generated "
                      "only up to an arity of 12.");
    };
} // namespace sys

#define _libcxxext_internal_aggregate_mark_primary_template 1

#define _libcxxext_internal_tparam_count 0
#define _libcxxext_internal_tparam_sub_one -1
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 1
#define _libcxxext_internal_tparam_sub_one 0
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 2
#define _libcxxext_internal_tparam_sub_one 1
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 3
#define _libcxxext_internal_tparam_sub_one 2
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 4
#define _libcxxext_internal_tparam_sub_one 3
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 5
#define _libcxxext_internal_tparam_sub_one 4
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 6
#define _libcxxext_internal_tparam_sub_one 5
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 7
#define _libcxxext_internal_tparam_sub_one 6
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 8
#define _libcxxext_internal_tparam_sub_one 7
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 9
#define _libcxxext_internal_tparam_sub_one 8
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 10
#define _libcxxext_internal_tparam_sub_one 9
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 11
#define _libcxxext_internal_tparam_sub_one 10
#include <inline/DeclareAggregate.inl>
#undef _libcxxext_internal_tparam_sub_one
#undef _libcxxext_internal_tparam_count
#define _libcxxext_internal_tparam_count 12
#define _libcxxext_internal_tparam_sub_one 11
#include <inline/DeclareAggregate.inl>
