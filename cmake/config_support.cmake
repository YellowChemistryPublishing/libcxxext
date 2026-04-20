function(mark_as_enum VAR)
    if(NOT "${${VAR}}" IN_LIST ARGN)
        set(ENUM_OPTIONS "")
        foreach(ENUM_OPT IN LISTS ARGN)
            list(APPEND ENUM_OPTIONS "\`${ENUM_OPT}\`")
        endforeach()
        list(JOIN ENUM_OPTIONS ", " ENUM_OPTIONS)

        message(FATAL_ERROR "`${VAR}` is `${${VAR}}`, expected one of: ${ENUM_OPTIONS}")
    endif()

    set_property(CACHE ${VAR} PROPERTY STRINGS ${ARGN})
endfunction()

macro(target_libcxxext_common_config TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 26
        CXX_STANDARD_REQUIRED OFF
        CXX_SCAN_FOR_MODULES OFF
        PREFIX ""
        IMPORT_PREFIX ""
    )
endmacro()

function(copy_dynlibs_to_build_target_dir TARGET_NAME)
    foreach(LIB_NAME IN LISTS ARGN)
        get_target_property(LIB_TYPE ${LIB_NAME} TYPE)
        if(NOT LIB_TYPE STREQUAL "SHARED_LIBRARY")
            continue()
        endif()

        add_custom_command(TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${LIB_NAME}>
            $<TARGET_FILE_DIR:${TARGET_NAME}>
        )
    endforeach()
endfunction()
