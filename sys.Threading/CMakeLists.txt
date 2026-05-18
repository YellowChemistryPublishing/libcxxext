if(NOT TARGET sys.Threading)
    file(GLOB SYS_THREAD_HEADERS CONFIGURE_DEPENDS "*.h")

    add_library(sys.Threading INTERFACE)
    target_libcxxext_common_config(sys.Threading)
    target_sources(sys.Threading INTERFACE
        FILE_SET HEADERS
        FILES ${SYS_THREAD_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Threading"
    )
    target_include_directories(sys.Threading INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    if(libcxxext_IS_TOP_LEVEL)
        generate_module_header(
            sys.Threading
            "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Threading"
            "${CMAKE_CURRENT_SOURCE_DIR}/"
            ${SYS_THREAD_HEADERS}
        )
    endif()
    if(LIBCXXEXT_EXPORT_PCH)
        target_precompile_headers(sys.Threading INTERFACE ${SYS_THREAD_HEADERS})
    endif()
    target_link_libraries(sys.Threading
        INTERFACE
        sys.PlatformSupport.Threading
        sys
        $<$<BOOL:${LIBCXXEXT_DEVELOPMENT_MODE}>:sys.BuildSupport.WarningsAsErrors>
        $<$<AND:$<CONFIG:Debug>,$<BOOL:${LIBCXXEXT_COVERAGE}>>:sys.BuildSupport.EnableCoverage>
    )

    if(LIBCXXEXT_DEVELOPMENT_MODE)
        target_lint_clang_tidy(sys.Threading "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"" ${SYS_THREAD_HEADERS})
    endif()

    install(TARGETS sys.Threading
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        FILE_SET HEADERS
    )
endif()
