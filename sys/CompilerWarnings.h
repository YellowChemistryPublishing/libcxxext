#pragma once

#include <Platform.h>

// NOLINTBEGIN(modernize-macro-to-enum)

/// @defgroup CompilerWarnings Compiler Warning Identifier Constants
/// @details Use with `_push_nowarn_*(...)` and `_pop_nowarn_*(...)`.

/// @addtogroup CompilerWarnings
/// @{

#define _clwarn_clang_bad_offsetof "-Winvalid-offsetof"
#define _clwarn_clang_nontrivial_memcpy "-Wsuspicious-memaccess"
#define _clwarn_clang_unused_param "-Wunused-parameter"
#define _clwarn_clang_pedantic "-Wpedantic"
#define _clwarn_clang_terminate "-Wterminate"
#define _clwarn_clang_use_after_free "-Wblock-capture-autoreleasing"
#define _clwarn_clang_literal_suffix "-Wuser-defined-literals"
#define _clwarn_clang_reserved_udl "-Wreserved-user-defined-literal"
#define _clwarn_clang_cast_align "-Wcast-align"
#define _clwarn_clang_unreachable "-Wunreachable-code-aggressive"
#define _clwarn_clang_nameless_struct_union "-Wnested-anon-types"
#define _clwarn_clang_anon_union_struct "-Wgnu-anonymous-struct"
#define _clwarn_clang_c_cast "-Wold-style-cast"
#define _clwarn_clang_deprecated "-Wdeprecated-declarations"
#define _clwarn_clang_zero_as_nullptr "-Wzero-as-null-pointer-constant"
#define _clwarn_clang_variadic_macro_args "-Wgnu-zero-variadic-macro-arguments"
#define _clwarn_clang_conversion "-Wimplicit-int-conversion"
#define _clwarn_clang_sign_conversion "-Wsign-conversion"
#define _clwarn_clang_float_conversion "-Wimplicit-float-conversion"
#define _clwarn_clang_int_float_conversion "-Wimplicit-int-float-conversion"
#define _clwarn_clang_double_promotion "-Wdouble-promotion"
#define _clwarn_clang_sign_compare "-Wsign-compare"
#define _clwarn_clang_implicit_fallthrough "-Wimplicit-fallthrough"

#define _clwarn_gcc_bad_offsetof "-Winvalid-offsetof"
#define _clwarn_gcc_nontrivial_memcpy "-Wclass-memaccess"
#define _clwarn_gcc_unused_param "-Wunused-parameter"
#define _clwarn_gcc_pedantic "-Wpedantic"
#define _clwarn_gcc_terminate "-Wterminate"
#define _clwarn_gcc_use_after_free "-Wuse-after-free"
#define _clwarn_gcc_literal_suffix "-Wliteral-suffix"
#define _clwarn_gcc_cast_align "-Wcast-align"
#define _clwarn_gcc_conversion "-Wconversion"
#define _clwarn_gcc_sign_conversion "-Wsign-conversion"
#define _clwarn_gcc_float_conversion "-Wfloat-conversion"
#define _clwarn_gcc_double_promotion "-Wdouble-promotion"
#define _clwarn_gcc_sign_compare "-Wsign-compare"
#define _clwarn_gcc_c_cast "-Wold-style-cast"
#define _clwarn_gcc_deprecated "-Wdeprecated-declarations"
#define _clwarn_gcc_zero_as_nullptr "-Wzero-as-null-pointer-constant"

#define _clwarn_msvc_literal_suffix 4455
#define _clwarn_msvc_unreachable 4702
#define _clwarn_msvc_nameless_struct_union 4201
#define _clwarn_msvc_export_interface 4251
#define _clwarn_msvc_overflow 4756
#define _clwarn_msvc_deprecated 4996

/// @}

/// @def _push_nowarn_clang(compilerWarning)
/// @brief Push clang nowarn.
#define _push_nowarn_clang(compilerWarning)
/// @def _push_nowarn_gcc(compilerWarning)
/// @brief Push gcc nowarn.
#define _push_nowarn_gcc(compilerWarning)
/// @def _push_nowarn_msvc(compilerWarning)
/// @brief Push msvc nowarn.
#define _push_nowarn_msvc(compilerWarning)
/// @def _pop_nowarn_clang()
/// @brief Pops clang nowarn.
#define _pop_nowarn_clang()
/// @def _pop_nowarn_gcc()
/// @brief Pops gcc nowarn.
#define _pop_nowarn_gcc()
/// @def _pop_nowarn_msvc()
/// @brief Pops msvc nowarn.
#define _pop_nowarn_msvc()

/// @def _clpragma_fwd(...)
/// @brief Forward a _Pragma to the compiler.
#define _clpragma_fwd(...) _Pragma(#__VA_ARGS__)

#if _libcxxext_compiler_clang

#undef _push_nowarn_clang
#define _push_nowarn_clang(compilerWarning)                 \
    _Pragma("clang diagnostic push");                       \
    _Pragma("clang diagnostic ignored \"-Wextra-semi\"");   \
    _clpragma_fwd(clang diagnostic ignored compilerWarning)

#undef _pop_nowarn_clang
#define _pop_nowarn_clang() _Pragma("clang diagnostic pop")

/// @def _push_nowarn_conv_comp()
/// @brief Push generic nowarn for built-in type conversion warnings.
#define _push_nowarn_conv_comp()                           \
    _push_nowarn_clang(_clwarn_clang_conversion);          \
    _push_nowarn_clang(_clwarn_clang_sign_conversion);     \
    _push_nowarn_clang(_clwarn_clang_float_conversion);    \
    _push_nowarn_clang(_clwarn_clang_double_promotion);    \
    _push_nowarn_clang(_clwarn_clang_sign_compare);        \
    _push_nowarn_clang(_clwarn_clang_int_float_conversion)
/// @def _pop_nowarn_conv_comp()
/// @brief Pops generic nowarn for built-in type conversion warnings.
#define _pop_nowarn_conv_comp() \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang()

/// @def _push_nowarn_deprecated()
/// @brief Push generic nowarn for deprecated warnings.
#define _push_nowarn_deprecated() _push_nowarn_clang(_clwarn_clang_deprecated)
/// @def _pop_nowarn_deprecated()
/// @brief Pops generic nowarn for deprecated warnings.
#define _pop_nowarn_deprecated() _pop_nowarn_clang()

/// @def _push_nowarn_c_cast()
/// @brief Push generic nowarn for C-style cast warnings.
#define _push_nowarn_c_cast() _push_nowarn_clang(_clwarn_clang_c_cast)
/// @def _pop_nowarn_c_cast()
/// @brief Pops generic nowarn for C-style cast warnings.
#define _pop_nowarn_c_cast() _pop_nowarn_clang()

#elif _libcxxext_compiler_gcc

#undef _push_nowarn_gcc
#define _push_nowarn_gcc(compilerWarning)                 \
    _Pragma("GCC diagnostic push");                       \
    _clpragma_fwd(GCC diagnostic ignored compilerWarning)

#undef _pop_nowarn_gcc
#define _pop_nowarn_gcc() _Pragma("GCC diagnostic pop")

#define _push_nowarn_conv_comp()                    \
    _push_nowarn_gcc(_clwarn_gcc_conversion);       \
    _push_nowarn_gcc(_clwarn_gcc_sign_conversion);  \
    _push_nowarn_gcc(_clwarn_gcc_float_conversion); \
    _push_nowarn_gcc(_clwarn_gcc_double_promotion); \
    _push_nowarn_gcc(_clwarn_gcc_sign_compare)
#define _pop_nowarn_conv_comp() \
    _pop_nowarn_gcc();          \
    _pop_nowarn_gcc();          \
    _pop_nowarn_gcc();          \
    _pop_nowarn_gcc();          \
    _pop_nowarn_gcc()

#define _push_nowarn_deprecated() _push_nowarn_gcc(_clwarn_gcc_deprecated)
#define _pop_nowarn_deprecated() _pop_nowarn_gcc()

#define _push_nowarn_c_cast() _push_nowarn_gcc(_clwarn_gcc_c_cast)
#define _pop_nowarn_c_cast() _push_nowarn_gcc(_clwarn_gcc_c_cast)

#elif _libcxxext_compiler_msvc

#undef _push_nowarn_msvc
#define _push_nowarn_msvc(compilerWarning)            \
    _Pragma("warning(push)");                         \
    _clpragma_fwd(warning(disable : compilerWarning))

#undef _pop_nowarn_msvc
#define _pop_nowarn_msvc() _Pragma("warning(pop)")

#define _push_nowarn_conv_comp() _push_nowarn_msvc(_clwarn_msvc_overflow)
#define _pop_nowarn_conv_comp() _pop_nowarn_msvc()

#define _push_nowarn_deprecated() _push_nowarn_msvc(_clwarn_msvc_deprecated)
#define _pop_nowarn_deprecated() _pop_nowarn_msvc()

#define _push_nowarn_c_cast()
#define _pop_nowarn_c_cast()

#endif

// NOLINTEND(modernize-macro-to-enum)
