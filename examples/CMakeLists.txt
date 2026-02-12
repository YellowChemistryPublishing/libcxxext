file(GLOB_RECURSE EXAMPLES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(EXAMPLE_FILE ${EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
    set(EXAMPLE_NAME "example_${EXAMPLE_NAME}")

    add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
    target_libcxxext_common_config(${EXAMPLE_NAME})
    target_link_libraries(${EXAMPLE_NAME}
        PRIVATE
        sys.BuildSupport.CompilerOptions
        sys sys.Containers sys.Threading
        $<$<BOOL:${LIBCXXEXT_COVERAGE}>:sys.BuildSupport.EnableCoverage>
    )
    set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "Examples")

    if(LIBCXXEXT_DEVELOPMENT_MODE)
        target_lint_clang_tidy(${EXAMPLE_NAME} "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"")
    endif()
endforeach()
