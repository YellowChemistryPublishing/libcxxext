cmake_minimum_required(VERSION 3.30)

project(libcxxext VERSION 1.0.0 LANGUAGES C CXX)

include(CMakeDependentOption)
include(cmake/config_support.cmake)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Config
if(NOT DEFINED LIBCXXEXT_DEVELOPMENT_MODE)
    set(LIBCXXEXT_DEVELOPMENT_MODE ${libcxxext_IS_TOP_LEVEL} CACHE BOOL "Build libcxxext in development mode.")
endif()
if(LIBCXXEXT_DEVELOPMENT_MODE AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install path prefix." FORCE)
endif()
cmake_dependent_option(LIBCXXEXT_EXAMPLES "Build examples." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
cmake_dependent_option(LIBCXXEXT_TESTS "Build tests." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
cmake_dependent_option(LIBCXXEXT_COVERAGE "Enable coverage instrumentation for debug builds (gcc/clang)." ON "LIBCXXEXT_DEVELOPMENT_MODE" OFF)
set(LIBCXXEXT_LIBRARY_TYPE "SHARED" CACHE STRING "Build type of libraries.")
mark_as_enum(LIBCXXEXT_LIBRARY_TYPE "SHARED" "STATIC")
set(LIBCXXEXT_EXPORT_PCH OFF CACHE BOOL "Use and produce precompiled headers.")
set(LIBCXXEXT_TEST_ASAN OFF CACHE BOOL "Enable AddressSanitizer for tests.")
set(LIBCXXEXT_TEST_UBSAN OFF CACHE BOOL "Enable UndefinedBehaviorSanitizer for tests.")

include(cmake/build_support.cmake)
include(cmake/clang_tidy.cmake)
include(cmake/gen_module_header.cmake)

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
    add_subdirectory(tests)

    find_program(GCOVR_EXECUTABLE gcovr)
    if(GCOVR_EXECUTABLE)
        add_custom_target(libcxxext_coverage
            COMMAND ${GCOVR_EXECUTABLE} --lcov "${CMAKE_BINARY_DIR}/coverage.info" --gcov-ignore-parse-errors negative_hits.warn "-f=(out|build|(sys[\\.A-Za-z]*))/" "-e=sys\\.Text/data/" "${CMAKE_BINARY_DIR}"
            VERBATIM
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        set_target_properties(libcxxext_coverage PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_BINARY_DIR}/coverage.info")
    endif()
endif()

set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_IGNORE_FILES "/[^/]*build/" "/__pycache__/" "/checks.json" "/Doxyfile" "/workflows/" "/sys.Text/data_unicode/" "/\\\\.[^/]+/")
#                                                            ^ Library consumers don't need.                                    ^ Dirs beginning with a dot, i.e. `.cache/`.
include(CPack)
