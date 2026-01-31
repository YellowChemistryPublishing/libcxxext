if(NOT Python3_EXECUTABLE)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
endif()

if(NOT RUN_CLANG_TIDY)
    find_program(RUN_CLANG_TIDY NAMES "clang-tidy-25" "clang-tidy-24" "clang-tidy-23" "clang-tidy-22" "clang-tidy-21" "clang-tidy-20" "clang-tidy-19" "clang-tidy" PATHS "C:/Program Files/LLVM/bin" "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/bin" "/usr/bin")
endif()
if(NOT RUN_CLANG_TIDY)
    message(WARNING "clang-tidy not found. Required for static analysis.")
else()
    set(CLANG_TIDY_ENVDIR "${CMAKE_CURRENT_LIST_DIR}/../workflows")
endif()

function(target_lint_clang_tidy TARGET_NAME)
    if(RUN_CLANG_TIDY)
        if(NOT TARGET clang_tidy_all)
            add_custom_target(clang_tidy_all)
        endif()

        get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)
        if((NOT TARGET_SOURCES OR TARGET_SOURCES STREQUAL "") AND ("${ARGN}" STREQUAL ""))
            message(WARNING "No sources provided, clang-tidy won't run!")
            return()
        endif()

        set(CLANG_TIDY_EXTRA_TOOL_ARGS)
        set(CLANG_TIDY_EXTRA_COMPILER_ARGS)

        set(FOUND_DASHDASH FALSE)
        foreach(ARG IN LISTS ARGN)
            if(ARG STREQUAL "--")
                set(FOUND_DASHDASH TRUE)
            elseif(NOT FOUND_DASHDASH)
                list(APPEND CLANG_TIDY_EXTRA_TOOL_ARGS ${ARG})
            else()
                list(APPEND CLANG_TIDY_EXTRA_COMPILER_ARGS ${ARG})
            endif()
        endforeach()

        add_custom_target(lint_ct_${TARGET_NAME}
            COMMAND
            ${CMAKE_COMMAND} -E env "PYTHONPATH=${CLANG_TIDY_ENVDIR}"
            ${Python3_EXECUTABLE} "${CLANG_TIDY_ENVDIR}/scripts/wrap_clang_tidy.py" ${RUN_CLANG_TIDY}
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<LIST:FILTER,$<TARGET_PROPERTY:${TARGET_NAME},SOURCES>,EXCLUDE,.*cmake_pch\.hxx.*>,PREPEND,\">,APPEND,\"> ${CLANG_TIDY_EXTRA_TOOL_ARGS}
            --
            -x c++ -std=c++$<TARGET_PROPERTY:${TARGET_NAME},CXX_STANDARD> -Wno-pragma-once-outside-header -Wno-pragma-system-header-outside-header
            #                                                                                             ^ When using `#pragma GCC system_header` with CMake precompiled headers.
            #                                                             ^ When running on a plain header, spurious.
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<LIST:FILTER,$<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>,EXCLUDE,^$>,PREPEND,\-I\">,APPEND,\">
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<LIST:FILTER,$<TARGET_PROPERTY:${TARGET_NAME},INTERFACE_INCLUDE_DIRECTORIES>,EXCLUDE,^$>,PREPEND,\-I\">,APPEND,\">
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<LIST:FILTER,$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_DEFINITIONS>,EXCLUDE,^$>,PREPEND,\"\-D>,APPEND,\">
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<LIST:FILTER,$<TARGET_PROPERTY:${TARGET_NAME},INTERFACE_COMPILE_DEFINITIONS>,EXCLUDE,^$>,PREPEND,\"\-D>,APPEND,\">
            ${CLANG_TIDY_EXTRA_COMPILER_ARGS}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND_EXPAND_LISTS
        )
        add_dependencies(clang_tidy_all lint_ct_${TARGET_NAME})
    endif()
endfunction()
