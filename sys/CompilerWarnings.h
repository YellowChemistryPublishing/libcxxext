#pragma once

/// @file

#include <Platform.h>

// NOLINTBEGIN(modernize-macro-to-enum)

/// @defgroup compiler_warnings Compiler Warning Identifier Constants
/// @brief Compiler-specific warning flag mappings.
/// @details Use with `_push_nowarn_*(...)` and `_pop_nowarn_*(...)`.

#include <data/ClWarnClang.h> // IWYU pragma: export
#include <data/ClWarnGCC.h>   // IWYU pragma: export
#include <data/ClWarnMSVC.h>  // IWYU pragma: export

/// @internal
/// @def _internal_pragma(...)
#define _internal_pragma(...) _Pragma(#__VA_ARGS__)

/// @def _nowarn_begin_clang()
/// @brief Begin suppressing `clang` warnings.
#define _nowarn_begin_clang()
/// @def _nowarn_begin_gcc()
/// @brief Begin suppressing `gcc` warnings.
#define _nowarn_begin_gcc()
/// @def _nowarn_begin_msvc()
/// @brief Begin suppressing `msvc` warnings.
#define _nowarn_begin_msvc()

/// @def _nowarn_one_clang(compilerWarning)
/// @brief Suppress one `clang` warning.
#define _nowarn_one_clang(compilerWarning)
/// @def _nowarn_one_gcc(compilerWarning)
/// @brief Suppress one `gcc` warning.
#define _nowarn_one_gcc(compilerWarning)
/// @def _nowarn_one_msvc(compilerWarning)
/// @brief Suppress one `msvc` warning.
#define _nowarn_one_msvc(compilerWarning)

/// @def _nowarn_begin_one_clang(compilerWarning)
/// @brief Begin suppressing one `clang` warning.
#define _nowarn_begin_one_clang(compilerWarning)
/// @def _nowarn_begin_one_gcc(compilerWarning)
/// @brief Begin suppressing one `gcc` warning.
#define _nowarn_begin_one_gcc(compilerWarning)
/// @def _nowarn_begin_one_msvc(compilerWarning)
/// @brief Begin suppressing one `msvc` warning.
#define _nowarn_begin_one_msvc(compilerWarning)

/// @def _nowarn_end_clang()
/// @brief End suppressing `clang` warnings.
#define _nowarn_end_clang()
/// @def _nowarn_end_gcc()
/// @brief End suppressing `gcc` warnings.
#define _nowarn_end_gcc()
/// @def _nowarn_end_msvc()
/// @brief End suppressing `msvc` warnings.
#define _nowarn_end_msvc()

/// @def _nowarn_begin_conv_comp()
/// @brief Begin suppressing generic built-in type conversion warnings.
#define _nowarn_begin_conv_comp()
/// @def _nowarn_end_conv_comp()
/// @brief End suppressing generic built-in type conversion warnings.
#define _nowarn_end_conv_comp()
/// @def _nowarn_begin_deprecated()
/// @brief Begin suppressing generic deprecated warnings.
#define _nowarn_begin_deprecated()
/// @def _nowarn_end_deprecated()
/// @brief End suppressing generic deprecated warnings.
#define _nowarn_end_deprecated()
/// @def _nowarn_begin_c_cast()
/// @brief Begin suppressing generic C-style cast warnings.
#define _nowarn_begin_c_cast()
/// @def _nowarn_end_c_cast()
/// @brief End suppressing generic C-style cast warnings.
#define _nowarn_end_c_cast()
/// @def _nowarn_begin_use_after_free()
/// @brief Begin suppressing generic use-after-free warnings.
#define _nowarn_begin_use_after_free()
/// @def _nowarn_end_use_after_free()
/// @brief End suppressing generic use-after-free warnings.
#define _nowarn_end_use_after_free()
/// @def _nowarn_begin_unreachable()
/// @brief Begin suppressing generic unreachable code warnings.
#define _nowarn_begin_unreachable()
/// @def _nowarn_end_unreachable()
/// @brief End suppressing generic unreachable code warnings.
#define _nowarn_end_unreachable()

#if _libcxxext_compiler_clang

#undef _nowarn_begin_clang
#define _nowarn_begin_clang() _internal_pragma(clang diagnostic push) _internal_pragma(clang diagnostic ignored "-Wextra-semi")
#undef _nowarn_one_clang
#define _nowarn_one_clang(compilerWarning) _internal_pragma(clang diagnostic ignored compilerWarning)
#undef _nowarn_begin_one_clang
#define _nowarn_begin_one_clang(compilerWarning) _nowarn_begin_clang() _nowarn_one_clang(compilerWarning)
#undef _nowarn_end_clang
#define _nowarn_end_clang() _internal_pragma(clang diagnostic pop)

#undef _nowarn_begin_conv_comp
#define _nowarn_begin_conv_comp()                                  \
    _nowarn_begin_clang();                                         \
    _nowarn_one_clang(_clwarn_clang_conversion);                   \
    _nowarn_one_clang(_clwarn_clang_sign_conversion);              \
    _nowarn_one_clang(_clwarn_clang_float_conversion);             \
    _nowarn_one_clang(_clwarn_clang_double_promotion);             \
    _nowarn_one_clang(_clwarn_clang_sign_compare);                 \
    _nowarn_one_clang(_clwarn_clang_implicit_int_float_conversion)
#undef _nowarn_end_conv_comp
#define _nowarn_end_conv_comp() _nowarn_end_clang()

#undef _nowarn_begin_deprecated
#define _nowarn_begin_deprecated()              \
    _nowarn_begin_clang();                      \
    _nowarn_one_clang(_clwarn_clang_deprecated)
#undef _nowarn_end_deprecated
#define _nowarn_end_deprecated() _nowarn_end_clang()

#undef _nowarn_begin_c_cast
#define _nowarn_begin_c_cast()                      \
    _nowarn_begin_clang();                          \
    _nowarn_one_clang(_clwarn_clang_old_style_cast)
#undef _nowarn_end_c_cast
#define _nowarn_end_c_cast() _nowarn_end_clang()

#undef _nowarn_begin_use_after_free
#define _nowarn_begin_use_after_free()                           \
    _nowarn_begin_clang();                                       \
    _nowarn_one_clang(_clwarn_clang_block_capture_autoreleasing)
#undef _nowarn_end_use_after_free
#define _nowarn_end_use_after_free() _nowarn_end_clang()

#undef _nowarn_begin_unreachable
#define _nowarn_begin_unreachable()                   \
    _nowarn_begin_clang();                            \
    _nowarn_one_clang(_clwarn_clang_unreachable_code)
#undef _nowarn_end_unreachable
#define _nowarn_end_unreachable() _nowarn_end_clang()

#elif _libcxxext_compiler_gcc

#undef _nowarn_begin_gcc
#define _nowarn_begin_gcc() _internal_pragma(GCC diagnostic push)
#undef _nowarn_one_gcc
#define _nowarn_one_gcc(compilerWarning) _internal_pragma(GCC diagnostic ignored compilerWarning)
#undef _nowarn_begin_one_gcc
#define _nowarn_begin_one_gcc(compilerWarning) _nowarn_begin_gcc() _nowarn_one_gcc(compilerWarning)
#undef _nowarn_end_gcc
#define _nowarn_end_gcc() _internal_pragma(GCC diagnostic pop)

#undef _nowarn_begin_conv_comp
#define _nowarn_begin_conv_comp()                  \
    _nowarn_begin_gcc();                           \
    _nowarn_one_gcc(_clwarn_gcc_conversion);       \
    _nowarn_one_gcc(_clwarn_gcc_sign_conversion);  \
    _nowarn_one_gcc(_clwarn_gcc_float_conversion); \
    _nowarn_one_gcc(_clwarn_gcc_double_promotion); \
    _nowarn_one_gcc(_clwarn_gcc_sign_compare)
#undef _nowarn_end_conv_comp
#define _nowarn_end_conv_comp() _nowarn_end_gcc()

#undef _nowarn_begin_deprecated
#define _nowarn_begin_deprecated()          \
    _nowarn_begin_gcc();                    \
    _nowarn_one_gcc(_clwarn_gcc_deprecated)
#undef _nowarn_end_deprecated
#define _nowarn_end_deprecated() _nowarn_end_gcc()

#undef _nowarn_begin_c_cast
#define _nowarn_begin_c_cast()          \
    _nowarn_begin_gcc();                \
    _nowarn_one_gcc(_clwarn_gcc_c_cast)
#undef _nowarn_end_c_cast
#define _nowarn_end_c_cast() _nowarn_end_gcc()

#undef _nowarn_begin_use_after_free
#define _nowarn_begin_use_after_free()          \
    _nowarn_begin_gcc();                        \
    _nowarn_one_gcc(_clwarn_gcc_use_after_free)
#undef _nowarn_end_use_after_free
#define _nowarn_end_use_after_free() _nowarn_end_gcc()

#elif _libcxxext_compiler_msvc

#undef _nowarn_begin_msvc
#define _nowarn_begin_msvc() _internal_pragma(warning(push))
#undef _nowarn_one_msvc
#define _nowarn_one_msvc(compilerWarning) _internal_pragma(warning(disable : compilerWarning))
#undef _nowarn_begin_one_msvc
#define _nowarn_begin_one_msvc(compilerWarning) _nowarn_begin_msvc() _nowarn_one_msvc(compilerWarning)
#undef _nowarn_end_msvc
#define _nowarn_end_msvc() _internal_pragma(warning(pop))

#undef _nowarn_begin_conv_comp
#define _nowarn_begin_conv_comp()                                                                                               \
    _nowarn_begin_msvc();                                                                                                       \
    _nowarn_one_msvc(_clwarn_msvc_overflow_in_floating_point_constant_arithmetic);                                              \
    _nowarn_one_msvc(_clwarn_msvc_operator_signed_integral_constant_overflow);                                                  \
    _nowarn_one_msvc(_clwarn_msvc_operator_overflow_in_constant_division_undefined_behavior);                                   \
    _nowarn_one_msvc(_clwarn_msvc_overflow_assigning_value_to_bit_field_that_can_only_hold_values_from_min_value_to_max_value); \
    _nowarn_one_msvc(_clwarn_msvc_overflow_in_constant_arithmetic);                                                             \
    _nowarn_one_msvc(_clwarn_msvc_identifier_conversion_from_type1_to_type2_possible_loss_of_data);                             \
    _nowarn_one_msvc(_clwarn_msvc_conversion_type_conversion_from_type1_to_type2_possible_loss_of_data);                        \
    _nowarn_one_msvc(_clwarn_msvc_variable_conversion_from_size_t_to_type_possible_loss_of_data);                               \
    _nowarn_one_msvc(_clwarn_msvc_conversion_truncation_from_type1_to_type2_4305);                                              \
    _nowarn_one_msvc(_clwarn_msvc_conversion_truncation_of_constant_value)
#undef _nowarn_end_conv_comp
#define _nowarn_end_conv_comp() _nowarn_end_msvc()

#undef _nowarn_begin_deprecated
#define _nowarn_begin_deprecated()                                            \
    _nowarn_begin_msvc();                                                     \
    _nowarn_one_msvc(_clwarn_msvc_deprecated_declaration_deprecation_message)
#undef _nowarn_end_deprecated
#define _nowarn_end_deprecated() _nowarn_end_msvc()

#undef _nowarn_begin_unreachable
#define _nowarn_begin_unreachable()                 \
    _nowarn_begin_msvc();                           \
    _nowarn_one_msvc(_clwarn_msvc_unreachable_code)
#undef _nowarn_end_unreachable
#define _nowarn_end_unreachable() _nowarn_end_msvc()

#endif

// NOLINTEND(modernize-macro-to-enum)
