if(NOT PLATFORM_SUPPORT_DIR)
    set(PLATFORM_SUPPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platform")
endif()

file(GLOB SYS_THREAD_HEADERS CONFIGURE_DEPENDS "*.h" "${PLATFORM_SUPPORT_DIR}/*.h")
file(GLOB SYS_THREAD_SRCS CONFIGURE_DEPENDS "*.cpp" "${PLATFORM_SUPPORT_DIR}/*.cpp")
add_library(sys.Threading ${LIBCXXEXT_LIBRARY_TYPE} ${SYS_THREAD_SRCS})
target_include_directories(sys.Threading PUBLIC "${PLATFORM_SUPPORT_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_precompile_headers(sys.Threading PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Threading")
target_link_libraries(sys.Threading PUBLIC sys sys.BuildSupport.CompilerOptions $<$<BOOL:${LIBCXXEXT_COVERAGE}>:sys.BuildSupport.EnableCoverage>)

if(LIBCXXEXT_DEVELOPMENT_MODE)
    target_link_libraries(sys.Threading PRIVATE sys.BuildSupport.WarningsAsErrors)

    target_lint_clang_tidy(sys.Threading "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/sys.Threading/.*\"" ${SYS_THREAD_HEADERS})
endif()
