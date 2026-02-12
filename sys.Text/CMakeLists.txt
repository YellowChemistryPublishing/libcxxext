file(GLOB_RECURSE SYS_TEXT_HEADERS CONFIGURE_DEPENDS "*.h")

add_library(sys.Text INTERFACE)
target_libcxxext_common_config(sys.Text)
target_sources(sys.Text INTERFACE
    FILE_SET HEADERS
    FILES ${SYS_TEXT_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Text"
)
target_include_directories(sys.Text INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
if(LIBCXXEXT_EXPORT_PCH)
    target_precompile_headers(sys.Text INTERFACE ${SYS_TEXT_HEADERS})
endif()
target_link_libraries(sys.Text INTERFACE
    sys
    $<$<BOOL:${LIBCXXEXT_DEVELOPMENT_MODE}>:sys.BuildSupport.WarningsAsErrors>
    $<$<BOOL:${LIBCXXEXT_COVERAGE}>:sys.BuildSupport.EnableCoverage>
)

if(LIBCXXEXT_DEVELOPMENT_MODE)
    target_lint_clang_tidy(sys.Text "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"" ${SYS_TEXT_HEADERS})
endif()

install(TARGETS sys.Text FILE_SET HEADERS)
