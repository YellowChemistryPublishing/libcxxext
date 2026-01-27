file(GLOB_RECURSE SYS_HEADERS CONFIGURE_DEPENDS "*.h")

add_library(sys INTERFACE)
target_include_directories(sys INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_precompile_headers(sys INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/module/sys")
target_link_libraries(sys INTERFACE sys.BuildSupport.CompilerOptions $<$<BOOL:${LIBCXXEXT_COVERAGE}>:sys.BuildSupport.EnableCoverage>)

if(LIBCXXEXT_DEVELOPMENT_MODE)
    target_link_libraries(sys INTERFACE sys.BuildSupport.WarningsAsErrors)

    target_lint_clang_tidy(sys "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/sys/.*\"" ${SYS_HEADERS})
endif()
