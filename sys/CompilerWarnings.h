#pragma once

#ifdef __clang__
#define _clWarn_bad_offsetof "-Winvalid-offsetof"
#define _clWarn_nontrivial_memcpy "-Wsuspicious-memaccess"
#define _clWarn_unused_param "-Wunused-parameter"
#define _clWarn_pedantic "-Wpedantic"
#define _clWarn_terminate "-Wterminate"
#define _clWarn_use_after_free "-Wblock-capture-autoreleasing"
#define _clWarn_literal_suffix "-Wuser-defined-literals"
#define _clWarn_cast_align ""
#define _clWarn_unreachable ""
#elifdef __GNUC__
#define _clWarn_bad_offsetof "-Winvalid-offsetof"
#define _clWarn_nontrivial_memcpy "-Wclass-memaccess"
#define _clWarn_unused_param "-Wunused-parameter"
#define _clWarn_pedantic "-Wpedantic"
#define _clWarn_terminate "-Wterminate"
#define _clWarn_use_after_free "-Wuse-after-free"
#define _clWarn_literal_suffix "-Wliteral-suffix"
#define _clWarn_cast_align "-Wcast-align"
#define _clWarn_unreachable ""
#elifdef _MSC_VER
#define _clWarn_msvc_dummy 4001
#define _clWarn_bad_offsetof _clWarn_msvc_dummy
#define _clWarn_nontrivial_memcpy _clWarn_msvc_dummy
#define _clWarn_unused_param _clWarn_msvc_dummy
#define _clWarn_pedantic _clWarn_msvc_dummy
#define _clWarn_terminate _clWarn_msvc_dummy
#define _clWarn_use_after_free _clWarn_msvc_dummy
#define _clWarn_literal_suffix 4455
#define _clWarn_cast_align _clWarn_msvc_dummy
#define _clWarn_unreachable 4702
#else
#define _clWarn_bad_offsetof
#define _clWarn_nontrivial_memcpy
#define _clWarn_unused_param
#define _clWarn_pedantic
#define _clWarn_terminate
#define _clWarn_use_after_free
#define _clWarn_literal_suffix
#define _clWarn_cast_align
#define _clWarn_unreachable
#endif

#define _clPragma_fwd(...) _Pragma(#__VA_ARGS__)
#if !defined(_MSC_VER) || !_MSC_VER
#define _push_nowarn(compilerWarning)                     \
    _Pragma("GCC diagnostic push");                       \
    _clPragma_fwd(GCC diagnostic ignored compilerWarning)
#define _pop_nowarn() _Pragma("GCC diagnostic pop")
#elif _MSC_VER
#define _push_nowarn(compilerWarning)                     \
    _Pragma("warning(push)");                       \
    _clPragma_fwd(warning(disable : compilerWarning))
#define _pop_nowarn() _Pragma("warning(pop)")
#else
#define _push_nowarn(compilerWarning)
#define _pop_nowarn()
#endif
