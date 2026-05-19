if(NOT TARGET sys.PlatformSupport.Alloc)
    file(GLOB_RECURSE SYS_SUP_ALLOC_HEADERS CONFIGURE_DEPENDS "*.h" "*.inl")

    add_library(sys.PlatformSupport.Alloc INTERFACE)
    target_libcxxext_common_config(sys.PlatformSupport.Alloc)
    target_sources(sys.PlatformSupport.Alloc INTERFACE
        FILE_SET HEADERS
        FILES ${SYS_SUP_ALLOC_HEADERS}
    )
    target_include_directories(sys.PlatformSupport.Alloc INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    if(LIBCXXEXT_EXPORT_PCH)
        target_precompile_headers(sys.PlatformSupport.Alloc INTERFACE ${SYS_SUP_ALLOC_HEADERS})
    endif()
    target_link_libraries(sys.PlatformSupport.Alloc INTERFACE
        sys
        $<$<BOOL:${LIBCXXEXT_DEVELOPMENT_MODE}>:sys.BuildSupport.WarningsAsErrors>
        $<$<AND:$<CONFIG:Debug>,$<BOOL:${LIBCXXEXT_COVERAGE}>>:sys.BuildSupport.EnableCoverage>
        $<$<CXX_COMPILER_ID:GNU>:stdc++exp>
    )

    if(LIBCXXEXT_DEVELOPMENT_MODE)
        target_lint_clang_tidy(sys.PlatformSupport.Alloc "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"" ${SYS_SUP_ALLOC_HEADERS})
    endif()

    install(TARGETS sys.PlatformSupport.Alloc FILE_SET HEADERS)
endif()
