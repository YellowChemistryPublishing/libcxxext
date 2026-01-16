file(GLOB_RECURSE SYS_CTRS_HEADERS CONFIGURE_DEPENDS "*.h")

add_library(sys.Containers INTERFACE)
target_include_directories(sys.Containers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_precompile_headers(sys.Containers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Containers")
target_link_libraries(sys.Containers INTERFACE sys.BuildSupport.CompilerOptions $<$<BOOL:LIBCXXEXT_COVERAGE>:sys.BuildSupport.EnableCoverage>)

if(LIBCXXEXT_DEVELOPMENT_MODE)
    target_link_libraries(sys.Containers INTERFACE sys.BuildSupport.WarningsAsErrors)

    target_lint_clang_tidy(sys.Containers "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/sys.Containers/.*\"" ${SYS_CTRS_HEADERS})
endif()
