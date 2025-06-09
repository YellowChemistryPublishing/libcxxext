add_library(sys.BuildSupport.CompilerWarnings INTERFACE)
target_compile_options(sys.BuildSupport.CompilerWarnings INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-Wsuggest-override> $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast> $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual> $<$<COMPILE_LANGUAGE:CXX>:-Wzero-as-null-pointer-constant>
    $<$<COMPILE_LANGUAGE:CXX>:-fconcepts-diagnostics-depth=4>

    -Wall -Wextra -pedantic -Wconversion -Wsign-conversion -Wdouble-promotion -Wunused-result -Wuninitialized -Wimplicit-fallthrough -Wcast-align -Wnull-dereference -Wformat=2 -Werror=format-security -Wno-psabi

    -fexceptions -fnon-call-exceptions -fasynchronous-unwind-tables
    
    -fno-math-errno -funsafe-math-optimizations -fno-rounding-math -fno-signed-zeros -fno-trapping-math -fexcess-precision=fast
    -fno-signaling-nans -fcx-limited-range

    -foptimize-sibling-calls # Force symmetric transfer optimization for coroutines.

    -fdata-sections -ffunction-sections
)

add_library(sys.BuildSupport.WarningsAsErrors INTERFACE)
target_compile_options(sys.BuildSupport.WarningsAsErrors INTERFACE -Werror)

add_library(sys.BuildSupport.Hardening INTERFACE)
target_compile_options(sys.BuildSupport.Hardening INTERFACE -fstack-protector-strong)
