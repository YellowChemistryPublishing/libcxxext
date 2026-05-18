if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang" OR MSVC))
    message(FATAL_ERROR "[libcxxext] Unsupported compiler!")
endif()

if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/EHs" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
endif()

add_library(sys.BuildSupport.CompilerOptions INTERFACE)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    target_compile_options(sys.BuildSupport.CompilerOptions INTERFACE
        $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU>,$<COMPILE_LANG_AND_ID:CXX,GNU>>:
        -fno-signaling-nans -fcx-limited-range -Wlogical-op -Wduplicated-cond -Wduplicated-branches
        -Wimplicit-fallthrough=5 -Walloc-zero -Wdangling-pointer=2 -Wmaybe-uninitialized>
        $<$<OR:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>>:
        -Wno-extra-semi -Wno-c++98-compat-extra-semi -Wno-c2y-extensions
        -Wimplicit-fallthrough -Wdocumentation -Wdangling -Wsometimes-uninitialized>
        $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fdiagnostics-all-candidates -fconcepts-diagnostics-depth=4 -Wnoexcept -Wnon-virtual-dtor -Wno-attributes>
        $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wconsumed -Wexceptions>
        # TODO(halloimdragon): Add `-Wunsafe-buffer-usage` for clang.

        $<$<COMPILE_LANGUAGE:CXX>:-Wsuggest-override -Wold-style-cast -Woverloaded-virtual -Wzero-as-null-pointer-constant -Wdeprecated-copy -Wredundant-move -Wrange-loop-construct>

        -finput-charset=UTF-8 -fexec-charset=UTF-8

        -Wall -Wextra -pedantic
        -fno-common -Wconversion -Wsign-conversion -Wdouble-promotion
        -Wundef -Wunused-result -Wcast-align -Wcast-qual -Wstrict-aliasing=2 -Wnon-virtual-dtor -Woverlength-strings -Wwrite-strings
        -Wmissing-declarations -Wmissing-include-dirs
        -Wuninitialized -Wnull-dereference
        -Wredundant-decls
        -Wformat=2 -Werror=format-security
        -Wno-psabi

        $<$<COMPILE_LANGUAGE:CXX>:-fexceptions -fnon-call-exceptions -fasynchronous-unwind-tables>

        -fno-math-errno -funsafe-math-optimizations -fno-rounding-math -fno-signed-zeros -fno-trapping-math -fexcess-precision=fast

        -foptimize-sibling-calls # Force symmetric transfer optimization for coroutines. Needed for embedded targets.

        -fdata-sections -ffunction-sections
    )
elseif(MSVC)
    target_compile_options(sys.BuildSupport.CompilerOptions INTERFACE
        /utf-8 /Zc:preprocessor $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/Zc:__cplusplus>
        /MP /Zf

        /permissive- /W4 /WX

        /fp:fast

        /wd4324 /wd5030

        /w14242 /w14254 /w14263 /w14265 /w14287 /w14296 /w14311 /w14545
        /w14546 /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 /w14905
        /w14906 /w14928
    )
endif()

add_library(sys.BuildSupport.WarningsAsErrors INTERFACE)
target_compile_options(sys.BuildSupport.WarningsAsErrors INTERFACE
    $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-Werror>
    $<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:-Wno-c99-extensions>
    $<$<OR:$<COMPILE_LANG_AND_ID:C,MSVC>,$<COMPILE_LANG_AND_ID:CXX,MSVC>>:/WX>
)

add_library(sys.BuildSupport.Hardening INTERFACE)
target_compile_options(sys.BuildSupport.Hardening INTERFACE
    $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:
    -D_FORTIFY_SOURCE=3 -fstack-protector-strong>
)

add_library(sys.BuildSupport.EnableCoverage INTERFACE)
target_compile_options(sys.BuildSupport.EnableCoverage INTERFACE
    $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-O0 -g -fprofile-arcs -ftest-coverage>
    $<$<OR:$<COMPILE_LANG_AND_ID:C,MSVC>,$<COMPILE_LANG_AND_ID:CXX,MSVC>>:/Zi /Od>
)
target_link_options(sys.BuildSupport.EnableCoverage INTERFACE
    $<$<OR:$<LINK_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<LINK_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fprofile-arcs>
    $<$<OR:$<LINK_LANG_AND_ID:C,MSVC>,$<LINK_LANG_AND_ID:CXX,MSVC>>:/DEBUG /PROFILE>
)

add_library(sys.BuildSupport.AddressSanitizer INTERFACE)
target_compile_options(sys.BuildSupport.AddressSanitizer INTERFACE
    $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fsanitize=address -fsanitize-address-use-after-scope>
    $<$<OR:$<COMPILE_LANG_AND_ID:C,MSVC>,$<COMPILE_LANG_AND_ID:CXX,MSVC>>:/fsanitize=address>
)
target_link_options(sys.BuildSupport.AddressSanitizer INTERFACE
    $<$<OR:$<LINK_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<LINK_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fsanitize=address -fsanitize-address-use-after-scope>
    $<$<OR:$<LINK_LANG_AND_ID:C,MSVC>,$<LINK_LANG_AND_ID:CXX,MSVC>>:/fsanitize=address>
)

add_library(sys.BuildSupport.UndefinedSanitizer INTERFACE)
target_compile_options(sys.BuildSupport.UndefinedSanitizer INTERFACE
    $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:
    -fsanitize=undefined,implicit-conversion,nullability -fsanitize-recover=undefined,float-cast-overflow,float-divide-by-zero>
)
target_link_options(sys.BuildSupport.UndefinedSanitizer INTERFACE
    $<$<OR:$<LINK_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<LINK_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:
    -fsanitize=undefined,implicit-conversion,nullability -fsanitize-recover=undefined,float-cast-overflow,float-divide-by-zero>
)

add_library(sys.BuildSupport.LeakSanitizer INTERFACE)
target_compile_options(sys.BuildSupport.LeakSanitizer INTERFACE
    $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fsanitize=leak>
)
target_link_options(sys.BuildSupport.LeakSanitizer INTERFACE
    $<$<OR:$<LINK_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<LINK_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fsanitize=leak>
)
