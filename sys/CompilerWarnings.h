#pragma once

#ifdef __clang__
#define _clWarn_bad_offsetof "-Winvalid-offsetof"
#define _clWarn_nontrivial_memcpy "-Wsuspicious-memaccess"
#define _clWarn_unused_param "-Wunused-parameter"
#define _clWarn_pedantic "-Wpedantic"
#define _clWarn_terminate "-Wterminate"
#define _clWarn_use_after_free "-Wblock-capture-autoreleasing"
#define _clWarn_literal_suffix "-Wuser-defined-literals"
#define _clWarn_clang_reserved_udl "-Wreserved-user-defined-literal"
#define _clWarn_cast_align "-Wcast-align"
#define _clWarn_unreachable "-Wunreachable-code-aggressive"
#define _clWarn_nameless_struct_union "-Wnested-anon-types"
#elifdef __GNUC__
#define _clWarn_gcc_dummy "-Wunknown-pragmas"
#define _clWarn_bad_offsetof "-Winvalid-offsetof"
#define _clWarn_nontrivial_memcpy "-Wclass-memaccess"
#define _clWarn_unused_param "-Wunused-parameter"
#define _clWarn_pedantic "-Wpedantic"
#define _clWarn_terminate "-Wterminate"
#define _clWarn_use_after_free "-Wuse-after-free"
#define _clWarn_literal_suffix "-Wliteral-suffix"
#define _clWarn_cast_align "-Wcast-align"
#define _clWarn_unreachable _clWarn_gcc_dummy
#define _clWarn_nameless_struct_union _clWarn_gcc_dummy
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
#define _clWarn_nameless_struct_union 4201
#else
#error "Unsupported compiler!"
#endif

#define _clPragma_fwd(...) _Pragma(#__VA_ARGS__)
#if defined(__clang__) && __clang__
#define _push_nowarn(compilerWarning)                       \
    _Pragma("clang diagnostic push");                       \
    _Pragma("clang diagnostic ignored \"-Wextra-semi\"");   \
    _clPragma_fwd(clang diagnostic ignored compilerWarning)
#define _pop_nowarn() _Pragma("clang diagnostic pop")
#elif defined(__GNUC__) && __GNUC__
#define _push_nowarn(compilerWarning)                     \
    _Pragma("GCC diagnostic push");                       \
    _clPragma_fwd(GCC diagnostic ignored compilerWarning)
#define _pop_nowarn() _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER) && _MSC_VER
#define _push_nowarn(compilerWarning)                 \
    _Pragma("warning(push)");                         \
    _clPragma_fwd(warning(disable : compilerWarning))
#define _pop_nowarn() _Pragma("warning(pop)")
#else
#error "Unsupported compiler!"
#endif
