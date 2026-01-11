#pragma once

#include <Platform.h>

#define _clWarn_clang_bad_offsetof "-Winvalid-offsetof"
#define _clWarn_clang_nontrivial_memcpy "-Wsuspicious-memaccess"
#define _clWarn_clang_unused_param "-Wunused-parameter"
#define _clWarn_clang_pedantic "-Wpedantic"
#define _clWarn_clang_terminate "-Wterminate"
#define _clWarn_clang_use_after_free "-Wblock-capture-autoreleasing"
#define _clWarn_clang_literal_suffix "-Wuser-defined-literals"
#define _clWarn_clang_reserved_udl "-Wreserved-user-defined-literal"
#define _clWarn_clang_cast_align "-Wcast-align"
#define _clWarn_clang_unreachable "-Wunreachable-code-aggressive"
#define _clWarn_clang_nameless_struct_union "-Wnested-anon-types"
#define _clWarn_clang_anon_union_struct "-Wgnu-anonymous-struct"
#define _clWarn_clang_c_cast "-Wold-style-cast"
#define _clWarn_clang_deprecated "-Wdeprecated-declarations"
#define _clWarn_clang_zero_as_nullptr "-Wzero-as-null-pointer-constant"
#define _clWarn_clang_variadic_macro_args "-Wgnu-zero-variadic-macro-arguments"
#define _clWarn_clang_conversion "-Wimplicit-int-conversion"
#define _clWarn_clang_sign_conversion "-Wsign-conversion"
#define _clWarn_clang_float_conversion "-Wimplicit-float-conversion"
#define _clWarn_clang_int_float_conversion "-Wimplicit-int-float-conversion"
#define _clWarn_clang_double_promotion "-Wdouble-promotion"
#define _clWarn_clang_sign_compare "-Wsign-compare"
#define _clWarn_clang_implicit_fallthrough "-Wimplicit-fallthrough"

#define _clWarn_gcc_bad_offsetof "-Winvalid-offsetof"
#define _clWarn_gcc_nontrivial_memcpy "-Wclass-memaccess"
#define _clWarn_gcc_unused_param "-Wunused-parameter"
#define _clWarn_gcc_pedantic "-Wpedantic"
#define _clWarn_gcc_terminate "-Wterminate"
#define _clWarn_gcc_use_after_free "-Wuse-after-free"
#define _clWarn_gcc_literal_suffix "-Wliteral-suffix"
#define _clWarn_gcc_cast_align "-Wcast-align"
#define _clWarn_gcc_conversion "-Wconversion"
#define _clWarn_gcc_sign_conversion "-Wsign-conversion"
#define _clWarn_gcc_float_conversion "-Wfloat-conversion"
#define _clWarn_gcc_double_promotion "-Wdouble-promotion"
#define _clWarn_gcc_sign_compare "-Wsign-compare"
#define _clWarn_gcc_c_cast "-Wold-style-cast"
#define _clWarn_gcc_deprecated "-Wdeprecated-declarations"
#define _clWarn_gcc_zero_as_nullptr "-Wzero-as-null-pointer-constant"

#define _clWarn_msvc_literal_suffix 4455
#define _clWarn_msvc_unreachable 4702
#define _clWarn_msvc_nameless_struct_union 4201
#define _clWarn_msvc_export_interface 4251
#define _clWarn_msvc_overflow 4756

#define _push_nowarn_clang(compilerWarning)
#define _push_nowarn_gcc(compilerWarning)
#define _push_nowarn_msvc(compilerWarning)
#define _pop_nowarn_clang()
#define _pop_nowarn_gcc()
#define _pop_nowarn_msvc()

#define _push_nowarn_conv_comp()
#define _pop_nowarn_conv_comp()

#define _clPragma_fwd(...) _Pragma(#__VA_ARGS__)

#if _libcxxext_compiler_clang

#undef _push_nowarn_clang
#define _push_nowarn_clang(compilerWarning)                 \
    _Pragma("clang diagnostic push");                       \
    _Pragma("clang diagnostic ignored \"-Wextra-semi\"");   \
    _clPragma_fwd(clang diagnostic ignored compilerWarning)

#undef _pop_nowarn_clang
#define _pop_nowarn_clang() _Pragma("clang diagnostic pop")

#undef _push_nowarn_conv_comp
#define _push_nowarn_conv_comp()                           \
    _push_nowarn_clang(_clWarn_clang_conversion);          \
    _push_nowarn_clang(_clWarn_clang_sign_conversion);     \
    _push_nowarn_clang(_clWarn_clang_float_conversion);    \
    _push_nowarn_clang(_clWarn_clang_double_promotion);    \
    _push_nowarn_clang(_clWarn_clang_sign_compare);        \
    _push_nowarn_clang(_clWarn_clang_int_float_conversion)

#undef _pop_nowarn_conv_comp
#define _pop_nowarn_conv_comp() \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang();        \
    _pop_nowarn_clang()

#undef _push_nowarn_c_cast
#define _push_nowarn_c_cast() _push_nowarn_clang(_clWarn_clang_c_cast)

#undef _pop_nowarn_c_cast
#define _pop_nowarn_c_cast() _push_nowarn_clang(_clWarn_clang_c_cast)

#elif _libcxxext_compiler_gcc

#undef _push_nowarn_gcc
#define _push_nowarn_gcc(compilerWarning)                 \
    _Pragma("GCC diagnostic push");                       \
    _clPragma_fwd(GCC diagnostic ignored compilerWarning)

#undef _pop_nowarn_gcc
#define _pop_nowarn_gcc() _Pragma("GCC diagnostic pop")

#undef _push_nowarn_conv_comp
#define _push_nowarn_conv_comp()                    \
    _push_nowarn_gcc(_clWarn_gcc_conversion);       \
    _push_nowarn_gcc(_clWarn_gcc_sign_conversion);  \
    _push_nowarn_gcc(_clWarn_gcc_float_conversion); \
    _push_nowarn_gcc(_clWarn_gcc_double_promotion); \
    _push_nowarn_gcc(_clWarn_gcc_sign_compare)

#undef _pop_nowarn_conv_comp
#define _pop_nowarn_conv_comp() \
    _pop_nowarn_gcc();          \
    _pop_nowarn_gcc();          \
    _pop_nowarn_gcc();          \
    _pop_nowarn_gcc();          \
    _pop_nowarn_gcc()

#undef _push_nowarn_c_cast
#define _push_nowarn_c_cast() _push_nowarn_gcc(_clWarn_gcc_c_cast)

#undef _pop_nowarn_c_cast
#define _pop_nowarn_c_cast() _push_nowarn_gcc(_clWarn_gcc_c_cast)

#elif _libcxxext_compiler_msvc

#undef _push_nowarn_msvc
#define _push_nowarn_msvc(compilerWarning)            \
    _Pragma("warning(push)");                         \
    _clPragma_fwd(warning(disable : compilerWarning))

#undef _pop_nowarn_msvc
#define _pop_nowarn_msvc() _Pragma("warning(pop)")

#undef _push_nowarn_conv_comp
#define _push_nowarn_conv_comp() _push_nowarn_msvc(_clWarn_msvc_overflow)

#undef _pop_nowarn_conv_comp
#define _pop_nowarn_conv_comp() _pop_nowarn_msvc()

#undef _push_nowarn_c_cast
#define _push_nowarn_c_cast()

#undef _pop_nowarn_c_cast
#define _pop_nowarn_c_cast()

#endif
