function(add_test_with_catch2 TEST_NAME TEST_FILE)
    set(TEST_NAME "test_${TEST_NAME}")

    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PUBLIC Catch2WithMain sys sys.Containers sys.Threading)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${TEST_NAME} PUBLIC stdc++exp)
    endif()

    add_test(${TEST_NAME} ${TEST_NAME})
endfunction()
