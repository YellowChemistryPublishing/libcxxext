cmake_minimum_required(VERSION 3.30)

project(libcxxext)

set(LIBRARY_TYPE "STATIC" CACHE STRING "Build type of library.")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SYS_SRCS CONFIGURE_DEPENDS
    "sys/*.h"
    "sys/*.cpp"
)
if (NOT PLATFORM_SUPPORT_RUNTIME_HEADER)
    set(PLATFORM_SUPPORT_RUNTIME_HEADER "${CMAKE_SOURCE_DIR}/sys/rt_required_generic.h")
endif()
if (PLATFORM_SUPPORT_RUNTIME_LIBRARY)
    list(APPEND SYS_SRCS ${PLATFORM_SUPPORT_RUNTIME_LIBRARY})
endif()

add_library(sys ${LIBRARY_TYPE} ${SYS_SRCS})
target_include_directories(sys PUBLIC "sys")
if(MSVC)
    target_compile_options(sys PUBLIC /FI "${PLATFORM_SUPPORT_RUNTIME_HEADER}")
else()
    target_compile_options(sys PUBLIC -include "${PLATFORM_SUPPORT_RUNTIME_HEADER}")
endif()
