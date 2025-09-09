#pragma once

#define _libcxxext_compiler_gcc 0
#define _libcxxext_compiler_clang 0
#define _libcxxext_compiler_msvc 0

#if defined(__clang__) && __clang__
#undef _libcxxext_compiler_clang
#define _libcxxext_compiler_clang 1
#elif defined(__GNUC__) && __GNUC__
#undef _libcxxext_compiler_gcc
#define _libcxxext_compiler_gcc 1
#elif defined(_MSC_VER) && _MSC_VER
#undef _libcxxext_compiler_msvc
#define _libcxxext_compiler_msvc 1
#else
#error "Unsupported compiler!"
#endif

#define _libcxxext_os_unknown 0
#define _libcxxext_os_aix 0
#define _libcxxext_os_android 0
#define _libcxxext_os_amigaos 0
#define _libcxxext_os_bsd 0
#define _libcxxext_os_ecos 0
#define _libcxxext_os_web 0
#define _libcxxext_os_fuchsia 0
#define _libcxxext_os_haiku 0
#define _libcxxext_os_os400 0
#define _libcxxext_os_illumos 0
#define _libcxxext_os_integrity 0
#define _libcxxext_os_linux 0
#define _libcxxext_os_lynx 0
#define _libcxxext_os_macos 0
#define _libcxxext_os_mint 0
#define _libcxxext_os_msdos 0
#define _libcxxext_os_nucleus 0
#define _libcxxext_os_qnx 0
#define _libcxxext_os_serenity 0
#define _libcxxext_os_syllable 0
#define _libcxxext_os_vxworks 0
#define _libcxxext_os_windows 0

#if defined(_AIX)
#undef _libcxxext_os_aix
#define _libcxxext_os_aix 1
#elif defined(__ANDROID__) && __ANDROID__
#undef _libcxxext_os_android
#define _libcxxext_os_android 1
#elif defined(AMIGA) || defined(__amigaos__)
#undef _libcxxext_os_amigaos
#define _libcxxext_os_amigaos 1
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__MidnightBSD__)
#undef _libcxxext_os_bsd
#define _libcxxext_os_bsd 1
#elif defined(__ECOS)
#undef _libcxxext_os_ecos
#define _libcxxext_os_ecos 1
#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#undef _libcxxext_os_web
#define _libcxxext_os_web 1
#elif defined(__Fuchsia__) && __Fuchsia__
#undef _libcxxext_os_fuchsia
#define _libcxxext_os_fuchsia 1
#elif defined(__HAIKU__)
#undef _libcxxext_os_haiku
#define _libcxxext_os_haiku 1
#elif defined(__OS400__)
#undef _libcxxext_os_os400
#define _libcxxext_os_os400 1
#elif defined(__illumos__)
#undef _libcxxext_os_illumos
#define _libcxxext_os_illumos 1
#elif defined(__INTEGRITY)
#undef _libcxxext_os_integrity
#define _libcxxext_os_integrity 1
#elif defined(__linux__)
#undef _libcxxext_os_linux
#define _libcxxext_os_linux 1
#elif defined(__Lynx__)
#undef _libcxxext_os_lynx
#define _libcxxext_os_lynx 1
#elif defined(__APPLE__) && defined(__MACH__) && __APPLE__ && __MACH__
#undef _libcxxext_os_macos
#define _libcxxext_os_macos 1
#elif defined(__MINT__)
#undef _libcxxext_os_mint
#define _libcxxext_os_mint 1
#elif defined(MSDOS) || defined(__MSDOS__) || defined(_MSDOS) || defined(__DOS__)
#undef _libcxxext_os_msdos
#define _libcxxext_os_msdos 1
#elif defined(__nucleus__)
#undef _libcxxext_os_nucleus
#define _libcxxext_os_nucleus 1
#elif defined(__QNX__)
#undef _libcxxext_os_qnx
#define _libcxxext_os_qnx 1
#elif defined(__serenity__)
#undef _libcxxext_os_serenity
#define _libcxxext_os_serenity 1
#elif defined(sun) || defined(__sun)
#elif defined(__SYLLABLE__)
#undef _libcxxext_os_syllable
#define _libcxxext_os_syllable 1
#elif defined(__VXWORKS__) || defined(__vxworks)
#undef _libcxxext_os_vxworks
#define _libcxxext_os_vxworks 1
#elif defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__) // I suppose in case we support other compilers in future.
#undef _libcxxext_os_windows
#define _libcxxext_os_windows 1
#else
#undef _libcxxext_os_unknown
#define _libcxxext_os_unknown 1
#endif
