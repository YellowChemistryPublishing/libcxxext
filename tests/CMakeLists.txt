include(../cmake/catch2.cmake)

set(CATCH_INSTALL_DOCS OFF CACHE INTERNAL "" FORCE)
set(CATCH_INSTALL_EXTRAS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(../Catch2 "${CMAKE_BINARY_DIR}/Catch2" EXCLUDE_FROM_ALL)
target_compile_features(Catch2 PUBLIC cxx_std_20)
target_compile_features(Catch2WithMain PUBLIC cxx_std_20)

add_subdirectory(../rapidcheck "${CMAKE_BINARY_DIR}/rapidcheck" EXCLUDE_FROM_ALL)
target_compile_options(rapidcheck PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-include;cstdint> $<$<CXX_COMPILER_ID:MSVC>:/w;/FI;cstdint>)

file(GLOB_RECURSE TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(TEST_FILE ${TESTS})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    set(TEST_NAME test_${TEST_NAME})

    add_test_with_catch2(${TEST_NAME} ${TEST_FILE})
    target_libcxxext_common_config(${TEST_NAME})
    copy_dynlibs_to_build_target_dir(${TEST_NAME} libsyscor sys sys.Containers sys.Text sys.Threading)
    target_link_libraries(${TEST_NAME} PRIVATE rapidcheck)
    set_tests_properties(${TEST_NAME} PROPERTIES ENVIRONMENT_MODIFICATION "RC_PARAMS=string_prepend:max_success=1000 max_size=1000 ")
    set_target_properties(${TEST_NAME} PROPERTIES FOLDER "Tests")

    if(LIBCXXEXT_DEVELOPMENT_MODE)
        target_lint_clang_tidy(${TEST_NAME} "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"")
    endif()
endforeach()
