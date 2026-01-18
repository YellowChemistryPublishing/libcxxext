find_program(RUN_CLANG_TIDY NAMES "clang-tidy-25" "clang-tidy-24" "clang-tidy-23" "clang-tidy-22" "clang-tidy-21" "clang-tidy-20" "clang-tidy-19" "clang-tidy" PATHS "C:/Program Files/LLVM/bin" "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/bin" "/usr/bin")
if(NOT RUN_CLANG_TIDY)
    message(WARNING "clang-tidy not found. Required for static analysis.")
endif()

function(target_lint_clang_tidy TARGET_NAME CLANG_TIDY_ARGS)
    if(RUN_CLANG_TIDY)
        if(NOT TARGET clang_tidy_all)
            add_custom_target(clang_tidy_all)
        endif()

        get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)
        if((NOT TARGET_SOURCES OR TARGET_SOURCES STREQUAL "") AND ("${ARGN}" STREQUAL ""))
            message(WARNING "No sources provided, clang-tidy won't run!")
            return()
        endif()

        add_custom_target(lint_ct_${TARGET_NAME}
            COMMAND
            ${RUN_CLANG_TIDY} $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<LIST:FILTER,$<TARGET_PROPERTY:${TARGET_NAME},SOURCES>,EXCLUDE,.*cmake_pch\.hxx.*>,PREPEND,\">,APPEND,\"> ${ARGN} ${CLANG_TIDY_ARGS} --
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>,PREPEND,\-I\">,APPEND,\">
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET_NAME},INTERFACE_INCLUDE_DIRECTORIES>,PREPEND,\-I\">,APPEND,\">
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_DEFINITIONS>,PREPEND,\-D>,APPEND,>
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET_NAME},INTERFACE_COMPILE_DEFINITIONS>,PREPEND,\-D>,APPEND,>
            -x c++ -std=c++26 -Wno-pragma-once-outside-header -Wno-pragma-system-header-outside-header
            #                                                 ^ When using `#pragma GCC system_header` with CMake precompiled headers.
            #                 ^ When running on a plain header, spurious.
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND_EXPAND_LISTS
        )
        add_dependencies(clang_tidy_all lint_ct_${TARGET_NAME})
    endif()
endfunction()
