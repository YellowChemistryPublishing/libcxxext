cmake_minimum_required(VERSION 3.30)

project(libcxxext)

include(CMakeDependentOption)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Config
if(NOT DEFINED LIBCXXEXT_DEVELOPMENT_MODE)
    set(LIBCXXEXT_DEVELOPMENT_MODE ${PROJECT_IS_TOP_LEVEL} CACHE BOOL "Build libcxxext in development mode.")
endif()
if(LIBCXXEXT_DEVELOPMENT_MODE AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install path prefix." FORCE)
endif()
cmake_dependent_option(LIBCXXEXT_EXAMPLES "Build examples." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
cmake_dependent_option(LIBCXXEXT_TESTS "Build tests." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
cmake_dependent_option(LIBCXXEXT_COVERAGE "Enable coverage instrumentation (gcc/clang)." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
set(LIBCXXEXT_LIBRARY_TYPE "STATIC" CACHE STRING "Build type of libraries.")
set(LIBCXXEXT_EXPORT_PCH ON CACHE BOOL "Use and produce precompiled headers.")
set(LIBCXXEXT_TEST_ASAN OFF CACHE BOOL "Enable AddressSanitizer for tests.")
set(LIBCXXEXT_TEST_UBSAN OFF CACHE BOOL "Enable UndefinedBehaviorSanitizer for tests.")

include(cmake/build_support.cmake)
include(cmake/clang_tidy.cmake)
include(cmake/target_common.cmake)

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
    target_compile_features(Catch2 PUBLIC cxx_std_20)
    target_compile_features(Catch2WithMain PUBLIC cxx_std_20)
    add_subdirectory(rapidcheck EXCLUDE_FROM_ALL)
    if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang|AppleClang)")
        target_compile_options(rapidcheck PRIVATE -include cstdint)
    elseif(MSVC)
        target_compile_options(rapidcheck PRIVATE /FI cstdint)
    endif()

    add_subdirectory(tests)
endif()
