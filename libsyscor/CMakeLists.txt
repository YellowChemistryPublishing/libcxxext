add_library(libsyscor ${LIBCXXEXT_LIBRARY_TYPE} "src.cpp")
target_libcxxext_common_config(libsyscor)
set_target_properties(libsyscor PROPERTIES
    VISIBILITY_INLINES_HIDDEN OFF
    C_VISIBILITY_PRESET default
    CXX_VISIBILITY_PRESET default
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

install(TARGETS libsyscor
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
