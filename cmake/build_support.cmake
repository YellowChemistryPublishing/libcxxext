add_library(sys.BuildSupport.CompilerWarnings INTERFACE)
if (NOT MSVC)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(sys.BuildSupport.CompilerWarnings INTERFACE
            $<$<COMPILE_LANGUAGE:CXX>:-fconcepts-diagnostics-depth=4>
            -fno-signaling-nans -fcx-limited-range
        )
    endif()

    target_compile_options(sys.BuildSupport.CompilerWarnings INTERFACE
        $<$<COMPILE_LANGUAGE:CXX>:-Wsuggest-override> $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast> $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual> $<$<COMPILE_LANGUAGE:CXX>:-Wzero-as-null-pointer-constant>

        -Wall -Wextra -pedantic -Wconversion -Wsign-conversion -Wdouble-promotion -Wunused-result -Wuninitialized -Wimplicit-fallthrough -Wcast-align -Wnull-dereference -Wformat=2 -Werror=format-security -Wno-psabi

        -fexceptions -fnon-call-exceptions -fasynchronous-unwind-tables
        
        -fno-math-errno -funsafe-math-optimizations -fno-rounding-math -fno-signed-zeros -fno-trapping-math -fexcess-precision=fast

        -foptimize-sibling-calls # Force symmetric transfer optimization for coroutines.

        -fdata-sections -ffunction-sections
    )
else()
    target_compile_options(sys.BuildSupport.CompilerWarnings INTERFACE
        /Wall
        /wd4820 # The type and order of elements caused the compiler to add padding to the end of a struct. See align for more information on padding in a struct.
        /wd4625 # A copy constructor was deleted or not accessible in a base class and was therefore not generated for a derived class. Any attempt to copy an object of this type will cause a compiler error.
        /wd4626 # An assignment operator was deleted or not accessible in a base class and was therefore not generated for a derived class. Any attempt to assign objects of this type will cause a compiler error.
        /wd5026 # Move constructor was implicitly defined as deleted.
        /wd5027 # Move assignment operator was implicitly defined as deleted.
        /wd4201 # Nonstandard extension used: nameless struct/union.
        /wd4514 # Unreferenced inline function has been removed.
        /wd4702 # Unreachable code.

        /EHa

        /fp:fast
    )
endif()

add_library(sys.BuildSupport.WarningsAsErrors INTERFACE)
if (NOT MSVC)
    target_compile_options(sys.BuildSupport.WarningsAsErrors INTERFACE -Werror)
else()
    target_compile_options(sys.BuildSupport.WarningsAsErrors INTERFACE /WX)
endif()

add_library(sys.BuildSupport.Hardening INTERFACE)
if (NOT MSVC)
    target_compile_options(sys.BuildSupport.Hardening INTERFACE -fstack-protector-strong)
endif()
