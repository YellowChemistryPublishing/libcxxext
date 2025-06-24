cmake_minimum_required(VERSION 3.30)

project(libcxxext)

include(CMakeDependentOption)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(LIBCXXEXT_DEVELOPMENT_MODE ON CACHE STRING "Build libcxxext in development mode.")
else()
    set(LIBCXXEXT_DEVELOPMENT_MODE OFF CACHE STRING "Build libcxxext in development mode.")
endif()

cmake_dependent_option(LIBCXXEXT_EXAMPLES "Build examples." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
cmake_dependent_option(LIBCXXEXT_TESTS "Build tests." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
set(LIBCXXEXT_LIBRARY_TYPE "STATIC" CACHE STRING "Build type of libraries.")

set(CMAKE_CXX_STANDARD 26)

include(cmake/build_support.cmake)
if (LIBCXXEXT_DEVELOPMENT_MODE)
    link_libraries(sys.BuildSupport.WarningsAsErrors)
endif()

find_program(RUN_CPPCHECK cppcheck PATHS "C:/Program Files/Cppcheck" "/usr/bin")
if (NOT RUN_CPPCHECK)
    message(FATAL_ERROR "Cppcheck not found. Required for static analysis.")
endif()
set(LINT_CPPCHECK ${RUN_CPPCHECK} --suppress=preprocessorErrorDirective --enable=all --inconclusive --inline-suppr --force --max-configs=64 -DconfigUSE_16_BIT_TICKS)

include(cmake/clang_tidy.cmake)

# sys

add_library(sys INTERFACE)
target_include_directories(sys INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/sys")
target_precompile_headers(sys INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/sys/module/sys")
target_link_libraries(sys INTERFACE sys.BuildSupport.CompilerOptions)

# sys.Containers

add_library(sys.Containers INTERFACE)
target_include_directories(sys.Containers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/sys.Containers")
target_precompile_headers(sys.Containers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/sys.Containers/module/sys.Containers")
target_link_libraries(sys.Containers INTERFACE sys.BuildSupport.CompilerOptions)

# sys.Threading

if (NOT PLATFORM_SUPPORT_DIR)
    set(PLATFORM_SUPPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sys.Threading/platform")
endif()
file(GLOB SYS_THREAD_SRCS CONFIGURE_DEPENDS
    "sys.Threading/*.h"
    "sys.Threading/*.cpp"
    "${PLATFORM_SUPPORT_DIR}/*.h"
    "${PLATFORM_SUPPORT_DIR}/*.cpp"
)

add_library(sys.Threading ${LIBCXXEXT_LIBRARY_TYPE} ${SYS_THREAD_SRCS})
target_include_directories(sys.Threading PUBLIC "${PLATFORM_SUPPORT_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/sys" "${CMAKE_CURRENT_SOURCE_DIR}/sys.Threading")
target_precompile_headers(sys.Threading PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/sys.Threading/module/sys.Threading")
target_link_libraries(sys.Threading INTERFACE sys.BuildSupport.CompilerOptions)

if (LIBCXXEXT_DEVELOPMENT_MODE)
    clang_tidy_target(sys.Threading "-header-filter=\"${CMAKE_CURRENT_SOURCE_DIR}/sys.Threading/.*\"")
endif()

# Examples

if (LIBCXXEXT_EXAMPLES)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        link_libraries(stdc++exp) # For `std::print(ln)(...)`.
    endif()

    file(GLOB EXAMPLES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")
    foreach(EXAMPLE_FILE ${EXAMPLES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
        set(EXAMPLE_NAME "example_${EXAMPLE_NAME}")

        add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
        target_link_libraries(${EXAMPLE_NAME} PUBLIC sys sys.Containers sys.Threading)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_link_libraries(${EXAMPLE_NAME} PUBLIC stdc++exp)
        endif()

        if (LIBCXXEXT_DEVELOPMENT_MODE)
            clang_tidy_target(${EXAMPLE_NAME} "-header-filter=\"${CMAKE_CURRENT_SOURCE_DIR}/examples/.*\"")
        endif()
    endforeach()
endif()

# Tests

if (LIBCXXEXT_TESTS)
    enable_testing()

    set(CATCH_INSTALL_DOCS OFF CACHE INTERNAL "" FORCE)
    set(CATCH_INSTALL_EXTRAS OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(Catch2)

    file(GLOB TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    foreach(TEST_FILE ${TESTS})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        set(TEST_NAME "test_${TEST_NAME}")

        add_executable(${TEST_NAME} ${TEST_FILE})
        target_link_libraries(${TEST_NAME} PUBLIC Catch2WithMain sys sys.Containers sys.Threading)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_link_libraries(${TEST_NAME} PUBLIC stdc++exp)
        endif()

        add_test(${TEST_NAME} ${TEST_NAME})
        
        if (LIBCXXEXT_DEVELOPMENT_MODE)
            clang_tidy_target(${TEST_NAME} "-header-filter=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/.*\"")
        endif()
    endforeach()
endif()
