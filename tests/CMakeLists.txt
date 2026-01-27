include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/catch2.cmake)

file(GLOB_RECURSE TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(TEST_FILE ${TESTS})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_test_with_catch2(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(test_${TEST_NAME} PRIVATE rapidcheck)

    if(LIBCXXEXT_DEVELOPMENT_MODE)
        target_lint_clang_tidy(test_${TEST_NAME} "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"")
    endif()
endforeach()
