file(GLOB_RECURSE SYS_HEADERS CONFIGURE_DEPENDS "*.h")

add_library(sys INTERFACE)
target_sources(sys INTERFACE
    FILE_SET HEADERS
    FILES ${SYS_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/module/sys"
)
target_include_directories(sys INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_precompile_headers(sys INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/module/sys")
target_link_libraries(sys INTERFACE
    sys.BuildSupport.CompilerOptions
    $<$<BOOL:${LIBCXXEXT_DEVELOPMENT_MODE}>:sys.BuildSupport.WarningsAsErrors>
    $<$<BOOL:${LIBCXXEXT_COVERAGE}>:sys.BuildSupport.EnableCoverage>
)

if(LIBCXXEXT_DEVELOPMENT_MODE)
    target_lint_clang_tidy(sys "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/sys/.*\"" ${SYS_HEADERS})
endif()

install(TARGETS sys FILE_SET HEADERS)
