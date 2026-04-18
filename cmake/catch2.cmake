function(add_test_with_catch2 TEST_NAME TEST_FILE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_libcxxext_common_config(${TEST_NAME})
    target_compile_options(${TEST_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-consumed>) # Doesn't play so well with Catch2.
    target_link_libraries(${TEST_NAME} PRIVATE
        Catch2WithMain
        sys.BuildSupport.CompilerOptions
        sys sys.Containers sys.Text sys.Threading
        $<$<BOOL:${LIBCXXEXT_TEST_ASAN}>:sys.BuildSupport.AddressSanitizer>
        $<$<BOOL:${LIBCXXEXT_TEST_UBSAN}>:sys.BuildSupport.UndefinedSanitizer>
        $<$<AND:$<CONFIG:Debug>,$<BOOL:${LIBCXXEXT_COVERAGE}>>:sys.BuildSupport.EnableCoverage>
    )

    add_test(${TEST_NAME} ${TEST_NAME})
endfunction()
