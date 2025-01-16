#
# Top-level CMakeLists.txt
#

cmake_minimum_required(VERSION 3.20)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Preventing in-source building. Please run cmake in a separate build directory.")
endif ()

#
# Project settings
#

project(
    TP2
    VERSION 24
    DESCRIPTION "Scheduler"
    LANGUAGES C)

set(CMAKE_C_STANDARD 17)

include(cmake/testing.cmake)
include(cmake/dependencies.cmake)

#
# Source
#

add_subdirectory(src)

#
# Testing
#

if (BUILD_TESTING)
    add_subdirectory(test)
endif ()
