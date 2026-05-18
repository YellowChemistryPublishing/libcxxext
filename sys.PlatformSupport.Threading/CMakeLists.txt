if(NOT TARGET sys.PlatformSupport.Threading)
    set(TINYCTHREAD_DISABLE_TESTS ON CACHE INTERNAL "" FORCE)
    set(TINYCTHREAD_INSTALL OFF CACHE INTERNAL "" FORCE)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
    add_subdirectory(../tinycthread "${CMAKE_BINARY_DIR}/tinycthread" EXCLUDE_FROM_ALL)

    file(GLOB_RECURSE SYS_SUP_THREAD_HEADERS CONFIGURE_DEPENDS "*.h" "*.inl")

    add_library(sys.PlatformSupport.Threading INTERFACE)
    target_libcxxext_common_config(sys.PlatformSupport.Threading)
    target_sources(sys.PlatformSupport.Threading INTERFACE
        FILE_SET HEADERS
        FILES ${SYS_SUP_THREAD_HEADERS}
    )
    target_include_directories(sys.PlatformSupport.Threading INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    if(LIBCXXEXT_EXPORT_PCH)
        target_precompile_headers(sys.PlatformSupport.Threading INTERFACE ${SYS_SUP_THREAD_HEADERS})
    endif()
    target_link_libraries(sys.PlatformSupport.Threading INTERFACE
        tinycthread
        sys
        $<$<BOOL:${LIBCXXEXT_DEVELOPMENT_MODE}>:sys.BuildSupport.WarningsAsErrors>
        $<$<AND:$<CONFIG:Debug>,$<BOOL:${LIBCXXEXT_COVERAGE}>>:sys.BuildSupport.EnableCoverage>
        $<$<CXX_COMPILER_ID:GNU>:stdc++exp>
    )

    if(LIBCXXEXT_DEVELOPMENT_MODE)
        target_lint_clang_tidy(sys.PlatformSupport.Threading "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"" ${SYS_SUP_THREAD_HEADERS})
    endif()

    install(TARGETS sys.PlatformSupport.Threading FILE_SET HEADERS)
endif()
