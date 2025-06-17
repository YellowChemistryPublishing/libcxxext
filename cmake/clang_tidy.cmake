find_program(RUN_CLANG_TIDY clang-tidy PATHS "C:/Program Files/LLVM/bin" "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/bin" "/usr/bin")
if (NOT RUN_CLANG_TIDY)
    message(FATAL_ERROR "clang-tidy not found. Required for static analysis.")
endif()

function(clang_tidy_target TARGET_NAME CLANG_TIDY_ARGS)
    if (NOT TARGET clang_tidy_all)
        add_custom_target(clang_tidy_all ALL)
    endif()

    get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)
    get_target_property(TARGET_INCLUDE_DIRS ${TARGET_NAME} INCLUDE_DIRECTORIES)
    get_target_property(TARGET_COMPILE_DEFS ${TARGET_NAME} COMPILE_DEFINITIONS)
    add_custom_target(clang_tidy_${TARGET_NAME} ALL
        ${CMAKE_COMMAND} "-DCLANG_TIDY_SOURCES=$<TARGET_PROPERTY:${TARGET_NAME},SOURCES>" "-DCLANG_TIDY_INCLUDE_DIRS=$<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>"
        "-DCLANG_TIDY_COMPILE_DEFS=$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_DEFINITIONS>"
        "-DCLANG_TIDY_COMMAND=${RUN_CLANG_TIDY}" "-DCLANG_TIDY_ARGS=${CLANG_TIDY_ARGS}" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang_tidy_script.cmake"
        VERBATIM
    )
    add_dependencies(clang_tidy_all clang_tidy_${TARGET_NAME})
endfunction()
