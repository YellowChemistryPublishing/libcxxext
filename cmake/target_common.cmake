macro(target_libcxxext_common_config TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 26
        CXX_STANDARD_REQUIRED OFF
        CXX_SCAN_FOR_MODULES OFF
        PREFIX ""
    )
endmacro()
