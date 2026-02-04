cmake_minimum_required(VERSION 3.30)

project(libcxxext)

include(CMakeDependentOption)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(LIBCXXEXT_DEVELOPMENT_MODE ON CACHE STRING "Build libcxxext in development mode.")
else()
    set(LIBCXXEXT_DEVELOPMENT_MODE OFF CACHE STRING "Build libcxxext in development mode.")
endif()

if(LIBCXXEXT_DEVELOPMENT_MODE AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install path prefix." FORCE)
endif()

# Config
cmake_dependent_option(LIBCXXEXT_EXAMPLES "Build examples." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
cmake_dependent_option(LIBCXXEXT_TESTS "Build tests." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
cmake_dependent_option(LIBCXXEXT_COVERAGE "Enable coverage instrumentation (gcc/clang)." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
set(LIBCXXEXT_LIBRARY_TYPE "STATIC" CACHE STRING "Build type of libraries.")
set(LIBCXXEXT_EXPORT_PCH ON CACHE BOOL "Use and produce precompiled headers.")
set(LIBCXXEXT_TEST_ASAN OFF CACHE BOOL "Enable AddressSanitizer for tests.")
set(LIBCXXEXT_TEST_UBSAN OFF CACHE BOOL "Enable UndefinedBehaviorSanitizer for tests.")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(cmake/build_support.cmake)
include(cmake/clang_tidy.cmake)

if(LIBCXXEXT_EXPORT_PCH)
    include(cmake/stdc.cmake)
    include(cmake/stdc++.cmake)
endif()

add_subdirectory(sys)
add_subdirectory(sys.Containers)
add_subdirectory(sys.Text)
add_subdirectory(sys.Threading)

# Examples
if(LIBCXXEXT_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(LIBCXXEXT_TESTS)
    enable_testing()

    set(CATCH_INSTALL_DOCS OFF CACHE INTERNAL "" FORCE)
    set(CATCH_INSTALL_EXTRAS OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(Catch2 EXCLUDE_FROM_ALL)
    add_subdirectory(rapidcheck EXCLUDE_FROM_ALL)

    add_subdirectory(tests)
endif()
