cmake_minimum_required(VERSION 3.30)

project(libcxxext)

set(LIBRARY_TYPE "STATIC" CACHE STRING "Build type of libraries." FORCE)

set(CMAKE_CXX_STANDARD 23)

add_library(sys INTERFACE)
target_include_directories(sys INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/sys")

add_library(sys.Containers INTERFACE)
target_include_directories(sys.Containers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/sys.Containers")

file(GLOB SYS_THREAD_SRCS CONFIGURE_DEPENDS
    "sys.Threading/*.h"
    "sys.Threading/*.cpp"
)
if (NOT PLATFORM_SUPPORT_THREADING_HEADER)
    set(PLATFORM_SUPPORT_THREADING_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/sys.Threading/rt_threading_generic.h")
    set(PLATFORM_SUPPORT_ADDITIONAL_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dep_concurrentqueue")
endif()
if (PLATFORM_SUPPORT_THREADING_LIBRARY)
    list(REMOVE_ITEM SYS_THREAD_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/sys.Threading/rt_threading_generic.cpp")
    list(APPEND SYS_THREAD_SRCS ${PLATFORM_SUPPORT_THREADING_LIBRARY})
endif()

add_library(sys.Threading ${LIBRARY_TYPE} ${SYS_THREAD_SRCS})
target_include_directories(sys.Threading PUBLIC sys sys.Threading ${PLATFORM_SUPPORT_ADDITIONAL_INCLUDE_DIRS})
if(MSVC)
    target_compile_options(sys.Threading PUBLIC /FI "${PLATFORM_SUPPORT_THREADING_HEADER}")
else()
    target_compile_options(sys.Threading PUBLIC -include "${PLATFORM_SUPPORT_THREADING_HEADER}")
endif()

file(GLOB EXAMPLES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")
foreach(EXAMPLE_FILE ${EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)

    add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
    target_link_libraries(${EXAMPLE_NAME} PUBLIC sys sys.Containers sys.Threading)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${EXAMPLE_NAME} PUBLIC stdc++exp)
    endif()
endforeach()
