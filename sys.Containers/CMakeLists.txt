file(GLOB_RECURSE SYS_CTRS_HEADERS CONFIGURE_DEPENDS "*.h")

add_library(sys.Containers INTERFACE)
target_libcxxext_common_config(sys.Containers)
target_sources(sys.Containers INTERFACE
    FILE_SET HEADERS
    FILES ${SYS_CTRS_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Containers"
)
target_include_directories(sys.Containers INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
if(LIBCXXEXT_EXPORT_PCH)
    target_precompile_headers(sys.Containers INTERFACE ${SYS_CTRS_HEADERS})
endif()
target_link_libraries(sys.Containers INTERFACE
    $<$<BOOL:${LIBCXXEXT_DEVELOPMENT_MODE}>:sys.BuildSupport.WarningsAsErrors>
    $<$<BOOL:${LIBCXXEXT_COVERAGE}>:sys.BuildSupport.EnableCoverage>
)

if(LIBCXXEXT_DEVELOPMENT_MODE)
    target_lint_clang_tidy(sys.Containers "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/sys.Containers/.*\"" ${SYS_CTRS_HEADERS})
endif()

install(TARGETS sys.Containers FILE_SET HEADERS)
