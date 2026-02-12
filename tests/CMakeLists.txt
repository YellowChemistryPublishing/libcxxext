include(${CMAKE_CURRENT_LIST_DIR}/../cmake/catch2.cmake)

file(GLOB_RECURSE TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(TEST_FILE ${TESTS})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    set(TEST_NAME test_${TEST_NAME})

    add_test_with_catch2(${TEST_NAME} ${TEST_FILE})
    target_libcxxext_common_config(${TEST_NAME})
    target_link_libraries(${TEST_NAME} PRIVATE rapidcheck)
    set_target_properties(${TEST_NAME} PROPERTIES FOLDER "Tests")

    if(LIBCXXEXT_DEVELOPMENT_MODE)
        target_lint_clang_tidy(${TEST_NAME} "-header-filter=\"^${CMAKE_CURRENT_SOURCE_DIR}/.*\"")
    endif()
endforeach()
