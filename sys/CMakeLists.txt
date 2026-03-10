file(GLOB_RECURSE SYS_HEADERS CONFIGURE_DEPENDS "*.h" "*.inl")

add_library(sys INTERFACE)
target_libcxxext_common_config(sys)
target_sources(sys INTERFACE
    FILE_SET HEADERS
    FILES ${SYS_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/module/sys"
)
target_include_directories(sys INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
if(PROJECT_IS_TOP_LEVEL)
    generate_module_header(
        sys
        "${CMAKE_CURRENT_SOURCE_DIR}/module/sys"
        "${CMAKE_CURRENT_SOURCE_DIR}/"
        ${SYS_HEADERS}
    )
endif()
if(LIBCXXEXT_EXPORT_PCH)
    target_precompile_headers(sys INTERFACE ${SYS_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/module/sys")
endif()
target_link_libraries(sys INTERFACE
    $<$<BOOL:${LIBCXXEXT_DEVELOPMENT_MODE}>:sys.BuildSupport.WarningsAsErrors>
    $<$<BOOL:${LIBCXXEXT_COVERAGE}>:sys.BuildSupport.EnableCoverage>
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(sys INTERFACE stdc++exp)
endif()

if(LIBCXXEXT_DEVELOPMENT_MODE)
    target_lint_clang_tidy(sys "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"" ${SYS_HEADERS})
endif()

install(TARGETS sys FILE_SET HEADERS)
