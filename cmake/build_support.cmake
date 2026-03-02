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
        $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fconcepts-diagnostics-depth=4>

        $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU>,$<COMPILE_LANG_AND_ID:CXX,GNU>>:-fno-signaling-nans -fcx-limited-range>
        $<$<OR:$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>>:-Wno-extra-semi -Wno-c++98-compat-extra-semi>

        $<$<COMPILE_LANGUAGE:CXX>:-Wsuggest-override> $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast> $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual> $<$<COMPILE_LANGUAGE:CXX>:-Wzero-as-null-pointer-constant>

        -finput-charset=UTF-8 -fexec-charset=UTF-8

        -Wall -Wextra -pedantic -Wconversion -Wsign-conversion -Wdouble-promotion -Wunused-result -Wuninitialized -Wimplicit-fallthrough -Wcast-align -Wnull-dereference -Wformat=2 -Werror=format-security -Wno-psabi

        $<$<COMPILE_LANGUAGE:CXX>:-fexceptions -fnon-call-exceptions -fasynchronous-unwind-tables>

        -fno-math-errno -funsafe-math-optimizations -fno-rounding-math -fno-signed-zeros -fno-trapping-math -fexcess-precision=fast

        -foptimize-sibling-calls # Force symmetric transfer optimization for coroutines. Needed for embedded targets.

        -fdata-sections -ffunction-sections
    )
elseif(MSVC)
    target_compile_options(sys.BuildSupport.CompilerOptions INTERFACE
        /utf-8 /Zc:preprocessor $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/Zc:__cplusplus>

        /permissive- /W4

        /fp:fast

        /wd4324
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
    $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fstack-protector-strong>
)

add_library(sys.BuildSupport.EnableCoverage INTERFACE)
target_compile_options(sys.BuildSupport.EnableCoverage INTERFACE
    $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-O0 -g -fprofile-arcs -ftest-coverage>
)
target_link_options(sys.BuildSupport.EnableCoverage INTERFACE
    $<$<OR:$<LINK_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<LINK_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fprofile-arcs>
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
    $<$<OR:$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fsanitize=undefined -fsanitize-recover=undefined,float-cast-overflow,float-divide-by-zero>
)
target_link_options(sys.BuildSupport.UndefinedSanitizer INTERFACE
    $<$<OR:$<LINK_LANG_AND_ID:C,GNU,Clang,AppleClang>,$<LINK_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fsanitize=undefined -fsanitize-recover=undefined,float-cast-overflow,float-divide-by-zero>
)
