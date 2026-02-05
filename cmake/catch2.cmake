function(add_test_with_catch2 TEST_NAME TEST_FILE)
    set(TEST_NAME "test_${TEST_NAME}")

    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE
        Catch2WithMain
        sys.BuildSupport.CompilerOptions
        sys sys.Containers sys.Text sys.Threading
        $<$<BOOL:${LIBCXXEXT_TEST_ASAN}>:sys.BuildSupport.MemorySanitizer>
        $<$<BOOL:${LIBCXXEXT_TEST_UBSAN}>:sys.BuildSupport.UndefinedSanitizer>
        $<$<BOOL:${LIBCXXEXT_COVERAGE}>:sys.BuildSupport.EnableCoverage>
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${TEST_NAME} PRIVATE stdc++exp)
    endif()

    add_test(${TEST_NAME} ${TEST_NAME})
endfunction()
