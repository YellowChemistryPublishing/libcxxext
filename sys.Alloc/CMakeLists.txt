if(NOT TARGET sys.Alloc)
    file(GLOB_RECURSE SYS_ALLOC_HEADERS CONFIGURE_DEPENDS "*.h" "*.inl")

    add_library(sys.Alloc INTERFACE)
    target_libcxxext_common_config(sys.Alloc)
    target_sources(sys.Alloc INTERFACE
        FILE_SET HEADERS
        FILES ${SYS_ALLOC_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Alloc"
    )
    target_include_directories(sys.Alloc INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    if(libcxxext_IS_TOP_LEVEL)
        generate_module_header(
            sys.Alloc
            "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Alloc"
            "${CMAKE_CURRENT_SOURCE_DIR}/"
            ${SYS_ALLOC_HEADERS}
        )
    endif()
    if(LIBCXXEXT_EXPORT_PCH)
        target_precompile_headers(sys.Alloc INTERFACE ${SYS_ALLOC_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Alloc")
    endif()
    target_link_libraries(sys.Alloc INTERFACE
        sys
        sys.PlatformSupport.Alloc
        $<$<BOOL:${LIBCXXEXT_DEVELOPMENT_MODE}>:sys.BuildSupport.WarningsAsErrors>
        $<$<AND:$<CONFIG:Debug>,$<BOOL:${LIBCXXEXT_COVERAGE}>>:sys.BuildSupport.EnableCoverage>
        $<$<CXX_COMPILER_ID:GNU>:stdc++exp>
    )

    if(LIBCXXEXT_DEVELOPMENT_MODE)
        target_lint_clang_tidy(sys.Alloc "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"" ${SYS_ALLOC_HEADERS})
    endif()

    install(TARGETS sys.Alloc FILE_SET HEADERS)
endif()
