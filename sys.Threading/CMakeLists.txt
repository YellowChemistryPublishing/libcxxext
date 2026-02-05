if(NOT PLATFORM_SUPPORT_DIR)
    set(PLATFORM_SUPPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platform")
endif()

file(GLOB_RECURSE SYS_THREAD_HEADERS CONFIGURE_DEPENDS "*.h" "${PLATFORM_SUPPORT_DIR}/*.h")
file(GLOB_RECURSE SYS_THREAD_SRCS CONFIGURE_DEPENDS "*.cpp" "${PLATFORM_SUPPORT_DIR}/*.cpp")
if(NOT PLATFORM_SUPPORT_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/platform")
    list(FILTER SYS_THREAD_HEADERS EXCLUDE REGEX ".*/rt_threading\\.h$")
    list(FILTER SYS_THREAD_SRCS EXCLUDE REGEX ".*/rt_threading\\.cpp$")
endif()

add_library(sys.Threading ${LIBCXXEXT_LIBRARY_TYPE} ${SYS_THREAD_SRCS})
target_sources(sys.Threading PUBLIC
    FILE_SET HEADERS
    FILES ${SYS_THREAD_HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/module/sys.Threading"
)
target_include_directories(sys.Threading PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PLATFORM_SUPPORT_DIR}>
)
if(LIBCXXEXT_EXPORT_PCH)
    target_precompile_headers(sys.Threading INTERFACE ${SYS_THREAD_HEADERS})
    target_link_libraries(sys.Threading PRIVATE sys.BuildSupport.stdc++inc)
endif()
target_link_libraries(sys.Threading
    PRIVATE
    sys.BuildSupport.CompilerOptions
    PUBLIC
    sys
    $<$<BOOL:${LIBCXXEXT_DEVELOPMENT_MODE}>:sys.BuildSupport.WarningsAsErrors>
    $<$<BOOL:${LIBCXXEXT_COVERAGE}>:sys.BuildSupport.EnableCoverage>
)
set_target_properties(sys.Threading PROPERTIES PREFIX "")

if(LIBCXXEXT_DEVELOPMENT_MODE)
    target_lint_clang_tidy(sys.Threading "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/sys.Threading/.*\"" ${SYS_THREAD_HEADERS})
endif()

install(TARGETS sys.Threading
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS
)
