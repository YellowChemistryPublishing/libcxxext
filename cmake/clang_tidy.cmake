find_program(RUN_CLANG_TIDY clang-tidy PATHS "C:/Program Files/LLVM/bin" "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/bin" "/usr/bin")
if (NOT RUN_CLANG_TIDY)
    message(WARNING "clang-tidy not found. Required for static analysis.")
endif()

function(target_lint_clang_tidy TARGET_NAME CLANG_TIDY_ARGS)
    if (RUN_CLANG_TIDY)
        if (NOT TARGET clang_tidy_all)
            add_custom_target(clang_tidy_all ALL)
        endif()
        set(CLANG_TIDY_EXTRA_ARGS ${ARGN})
        list(LENGTH CLANG_TIDY_EXTRA_ARGS CLANG_TIDY_EXTRA_ARGS_COUNT)
        if (CLANG_TIDY_EXTRA_ARGS_COUNT GREATER_EQUAL 0)
            list(GET CLANG_TIDY_EXTRA_ARGS 0 CLANG_TIDY_DRIVER_ARGS)
        endif()

        get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)
        if (NOT TARGET_SOURCES OR TARGET_SOURCES STREQUAL "")
            message(WARNING "No sources provided, clang-tidy won't run!")
            return()
        endif()

        if (NOT CLANG_TIDY_DRIVER_ARGS)
            set(CLANG_TIDY_DRIVER_ARGS "")
        endif()
        if (NOT CLANG_TIDY_CXX_STANDARD)
            set(CLANG_TIDY_CXX_STANDARD "c++26")
        endif()

        add_custom_target(lint_ct_${TARGET_NAME}
            ${RUN_CLANG_TIDY} $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET_NAME},SOURCES>,PREPEND,\">,APPEND,\"> ${CLANG_TIDY_ARGS} --
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>,PREPEND,\-I\">,APPEND,\">
            $<LIST:TRANSFORM,$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_DEFINITIONS>,PREPEND,\-D>,APPEND,>
            -x c++ -std=${CLANG_TIDY_CXX_STANDARD} -Wno-pragma-once-outside-header -Wno-pragma-system-header-outside-header ${CLANG_TIDY_DRIVER_ARGS}
            #                                                                      ^ When using `#pragma GCC system_header` with CMake precompiled headers.
            #                                      ^ When running on a plain header, spurious.
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND_EXPAND_LISTS
        )
        add_dependencies(${TARGET_NAME} lint_ct_${TARGET_NAME})
        add_dependencies(clang_tidy_all lint_ct_${TARGET_NAME})
    endif()
endfunction()
