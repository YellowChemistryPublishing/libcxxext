add_library(sys.BuildSupport.CompilerOptions INTERFACE)

if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang" OR MSVC))
    message(FATAL_ERROR "Unsupported compiler!")
endif()

if(NOT MSVC)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(sys.BuildSupport.CompilerOptions INTERFACE
            $<$<COMPILE_LANGUAGE:CXX>:-fconcepts-diagnostics-depth=4>
            -fno-signaling-nans -fcx-limited-range
        )
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(sys.BuildSupport.CompilerOptions INTERFACE
            -Wno-extra-semi -Wno-c++98-compat-extra-semi # Let us put semicolons after `_Pragma(...)`.
        )
    endif()

    target_compile_options(sys.BuildSupport.CompilerOptions INTERFACE
        $<$<COMPILE_LANGUAGE:CXX>:-Wsuggest-override> $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast> $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual> $<$<COMPILE_LANGUAGE:CXX>:-Wzero-as-null-pointer-constant>

        -finput-charset=UTF-8 -fexec-charset=UTF-8

        -Wall -Wextra -pedantic -Wconversion -Wsign-conversion -Wdouble-promotion -Wunused-result -Wuninitialized -Wimplicit-fallthrough -Wcast-align -Wnull-dereference -Wformat=2 -Werror=format-security -Wno-psabi

        -fexceptions -fnon-call-exceptions -fasynchronous-unwind-tables

        -fno-math-errno -funsafe-math-optimizations -fno-rounding-math -fno-signed-zeros -fno-trapping-math -fexcess-precision=fast

        -foptimize-sibling-calls # Force symmetric transfer optimization for coroutines. Needed for embedded targets.

        -fdata-sections -ffunction-sections
    )
else()
    target_compile_options(sys.BuildSupport.CompilerOptions INTERFACE
        /utf-8 /Zc:preprocessor

        /W4

        /EHa

        /fp:fast

        /wd4324
    )
endif()

add_library(sys.BuildSupport.WarningsAsErrors INTERFACE)
if(NOT MSVC)
    target_compile_options(sys.BuildSupport.WarningsAsErrors INTERFACE -Werror)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(sys.BuildSupport.WarningsAsErrors INTERFACE -Wno-c99-extensions)
    endif()
else()
    target_compile_options(sys.BuildSupport.WarningsAsErrors INTERFACE /WX)
endif()

add_library(sys.BuildSupport.Hardening INTERFACE)
if(NOT MSVC)
    target_compile_options(sys.BuildSupport.Hardening INTERFACE -fstack-protector-strong)
endif()

add_library(sys.BuildSupport.EnableCoverage INTERFACE)
if(NOT MSVC)
    target_compile_options(sys.BuildSupport.EnableCoverage INTERFACE -O0 -g -fprofile-arcs -ftest-coverage)
    target_link_options(sys.BuildSupport.EnableCoverage INTERFACE -fprofile-arcs)
endif()
