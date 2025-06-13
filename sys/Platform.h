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
