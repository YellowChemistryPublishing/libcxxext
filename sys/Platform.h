#pragma once

/// @file

/// @def _libcxxext_have_cxx_exceptions
/// @brief Whether the compiler is setup to provide runtime support for C++ exceptions.
#define _libcxxext_have_cxx_exceptions 0
#if (defined(__EXCEPTIONS) && __EXCEPTIONS) || (defined(_CPPUNWIND) && _CPPUNWIND)
#undef _libcxxext_have_cxx_exceptions
#define _libcxxext_have_cxx_exceptions 1
#endif

/// @defgroup compiler_detection Compiler Detection Constants
/// @brief `#ifdef`able constants for compiler detection.
/// @{

/// @def _libcxxext_compiler_gcc
/// @brief Check if the compiler is GCC.
#define _libcxxext_compiler_gcc 0
/// @def _libcxxext_compiler_clang
/// @brief Check if the compiler is Clang.
#define _libcxxext_compiler_clang 0
/// @def _libcxxext_compiler_msvc
/// @brief Check if the compiler is MSVC.
#define _libcxxext_compiler_msvc 0

/// @}

/// @cond
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
/// @endcond

/// @defgroup isa_detection Instruction Set Architecture Detection Constants
/// @brief `#ifdef`able constants for executing ISA detection.
/// @{

/// @def _libcxxext_arch_unknown
/// @brief Check if the executing ISA couldn't be resolved.
#define _libcxxext_arch_unknown 0
/// @def _libcxxext_arch_x86_64
/// @brief Check if the executing ISA is x86_64.
#define _libcxxext_arch_x86_64 0
/// @def _libcxxext_arch_aarch64
/// @brief Check if the executing ISA is AArch64.
#define _libcxxext_arch_aarch64 0
/// @def _libcxxext_arch_arm
/// @brief Check if the executing ISA is ARM32.
#define _libcxxext_arch_arm 0
/// @def _libcxxext_arch_blackfin
/// @brief Check if the executing ISA is Blackfin.
#define _libcxxext_arch_blackfin 0
/// @def _libcxxext_arch_x86
/// @brief Check if the executing ISA is x86 (32-bit).
#define _libcxxext_arch_x86 0
/// @def _libcxxext_arch_ia64
/// @brief Check if the executing ISA is IA-64.
#define _libcxxext_arch_ia64 0
/// @def _libcxxext_arch_loongarch
/// @brief Check if the executing ISA is LoongArch.
#define _libcxxext_arch_loongarch 0
/// @def _libcxxext_arch_mips
/// @brief Check if the executing ISA is MIPS.
#define _libcxxext_arch_mips 0
/// @def _libcxxext_arch_powerpc
/// @brief Check if the executing ISA is PowerPC.
#define _libcxxext_arch_powerpc 0
/// @def _libcxxext_arch_riscv
/// @brief Check if the executing ISA is RISC-V.
#define _libcxxext_arch_riscv 0
/// @def _libcxxext_arch_sparc
/// @brief Check if the executing ISA is SPARC.
#define _libcxxext_arch_sparc 0
/// @def _libcxxext_arch_superh
/// @brief Check if the executing ISA is SuperH.
#define _libcxxext_arch_superh 0

/// @}

/// @cond
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#undef _libcxxext_arch_x86_64
#define _libcxxext_arch_x86_64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#undef _libcxxext_arch_aarch64
#define _libcxxext_arch_aarch64 1
#elif defined(__arm__) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB) || defined(_ARM) || defined(_M_ARM) || defined(_M_ARMT) || defined(__arm)
#undef _libcxxext_arch_arm
#define _libcxxext_arch_arm 1
#elif defined(__bfin) || defined(__BFIN__) || defined(__ADSPBLACKFIN__)
#undef _libcxxext_arch_blackfin
#define _libcxxext_arch_blackfin 1
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__IA32__) || defined(_M_IX86) || defined(__X86__) || defined(_X86_) || defined(__THW_INTEL__) || \
    (defined(__I86__) && __I86__) || defined(__INTEL__) || defined(__386)
#undef _libcxxext_arch_x86
#define _libcxxext_arch_x86 1
#elif defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(__ia64) || defined(_M_IA64) || defined(__itanium__)
#undef _libcxxext_arch_ia64
#define _libcxxext_arch_ia64 1
#elif defined(__loongarch__)
#undef _libcxxext_arch_loongarch
#define _libcxxext_arch_loongarch 1
#elif defined(__mips__) || defined(mips) || (defined(__mips) && __mips) || defined(__MIPS__)
#undef _libcxxext_arch_mips
#define _libcxxext_arch_mips 1
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__ppc64__) || defined(__PPC__) || \
    defined(__PPC64__) || defined(_ARCH_PPC) || defined(_ARCH_PPC64) || defined(_M_PPC) || defined(_ARCH_PPC) || defined(_ARCH_PPC64) || defined(__ppc)
#undef _libcxxext_arch_powerpc
#define _libcxxext_arch_powerpc 1
#elif defined(__riscv)
#undef _libcxxext_arch_riscv
#define _libcxxext_arch_riscv 1
#elif defined(__sparc__) || defined(__sparc) || defined(__sparc_v8__) || defined(__sparc_v9__) || defined(__sparcv8) || defined(__sparcv9)
#undef _libcxxext_arch_sparc
#define _libcxxext_arch_sparc 1
#elif defined(__sh__)
#undef _libcxxext_arch_superh
#define _libcxxext_arch_superh 1
#else
#undef _libcxxext_arch_unknown
#define _libcxxext_arch_unknown 1
#endif
/// @endcond

/// @defgroup os_detection Operating System Detection Constants
/// @brief `#ifdef`able constants for target operating system detection.
/// @{

/// @def _libcxxext_os_unknown
/// @brief Check if the target OS couldn't be resolved.
#define _libcxxext_os_unknown 0
/// @def _libcxxext_os_aix
/// @brief Check if the target OS is AIX.
#define _libcxxext_os_aix 0
/// @def _libcxxext_os_android
/// @brief Check if the target OS is Android.
#define _libcxxext_os_android 0
/// @def _libcxxext_os_amigaos
/// @brief Check if the target OS is AmigaOS.
#define _libcxxext_os_amigaos 0
/// @def _libcxxext_os_bsd
/// @brief Check if the target OS is BSD.
#define _libcxxext_os_bsd 0
/// @def _libcxxext_os_ecos
/// @brief Check if the target OS is ECOS.
#define _libcxxext_os_ecos 0
/// @def _libcxxext_os_web
/// @brief Check if the target OS is Web.
#define _libcxxext_os_web 0
/// @def _libcxxext_os_fuchsia
/// @brief Check if the target OS is Fuchsia.
#define _libcxxext_os_fuchsia 0
/// @def _libcxxext_os_haiku
/// @brief Check if the target OS is Haiku.
#define _libcxxext_os_haiku 0
/// @def _libcxxext_os_os400
/// @brief Check if the target OS is OS400.
#define _libcxxext_os_os400 0
/// @def _libcxxext_os_illumos
/// @brief Check if the target OS is Illumos.
#define _libcxxext_os_illumos 0
/// @def _libcxxext_os_integrity
/// @brief Check if the target OS is Integrity.
#define _libcxxext_os_integrity 0
/// @def _libcxxext_os_linux
/// @brief Check if the target OS is Linux.
#define _libcxxext_os_linux 0
/// @def _libcxxext_os_lynx
/// @brief Check if the target OS is Lynx.
#define _libcxxext_os_lynx 0
/// @def _libcxxext_os_macos
/// @brief Check if the target OS is macOS.
#define _libcxxext_os_macos 0
/// @def _libcxxext_os_mint
/// @brief Check if the target OS is MinT.
#define _libcxxext_os_mint 0
/// @def _libcxxext_os_msdos
/// @brief Check if the target OS is MS-DOS.
#define _libcxxext_os_msdos 0
/// @def _libcxxext_os_nucleus
/// @brief Check if the target OS is Nucleus.
#define _libcxxext_os_nucleus 0
/// @def _libcxxext_os_qnx
/// @brief Check if the target OS is QNX.
#define _libcxxext_os_qnx 0
/// @def _libcxxext_os_serenity
/// @brief Check if the target OS is Serenity.
#define _libcxxext_os_serenity 0
/// @def _libcxxext_os_syllable
/// @brief Check if the target OS is Syllable.
#define _libcxxext_os_syllable 0
/// @def _libcxxext_os_vxworks
/// @brief Check if the target OS is VxWorks.
#define _libcxxext_os_vxworks 0
/// @def _libcxxext_os_windows
/// @brief Check if the target OS is Windows.
#define _libcxxext_os_windows 0

/// @}

// NOLINTBEGIN(readability-use-concise-preprocessor-directives)

/// @cond
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
#elif defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#undef _libcxxext_os_windows
#define _libcxxext_os_windows 1
#else
#undef _libcxxext_os_unknown
#define _libcxxext_os_unknown 1
#endif
/// @endcond

// NOLINTEND(readability-use-concise-preprocessor-directives)
