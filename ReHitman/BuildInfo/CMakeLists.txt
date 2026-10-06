cmake_minimum_required(VERSION 4.0)

# Options
option(USE_ORIGINAL_BUILDTAG "Use the hardcoded original build tag instead of generating a new one" OFF)
set(PROJECT_VERSION_OVERRIDE "" CACHE STRING "Override project version from CI")

# Override version
if(PROJECT_VERSION_OVERRIDE)
    set(BUILDINFO_VERSION "${PROJECT_VERSION_OVERRIDE}")
else()
    set(BUILDINFO_VERSION "1.0.0")
endif()

# Declare us
project(BuildInfo VERSION ${BUILDINFO_VERSION} LANGUAGES CXX)

# Generate build tag
if(USE_ORIGINAL_BUILDTAG)
    set(BUILDTAG "blood-build3-20060616-26123")
else()
    string(TIMESTAMP CURRENT_DATE "%Y%m%d")
    string(TIMESTAMP CURRENT_TIME "%H%M")
    set(BUILDTAG "blood-build3-${CURRENT_DATE}-${CURRENT_TIME}")
endif()

set(BUILDTAG "${BUILDTAG}" CACHE STRING "Build tag")

# Collect build info
set(BUILD_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
set(BUILD_PLATFORM "${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_PROCESSOR})")
if(NOT CMAKE_BUILD_TYPE)
    set(BUILD_TYPE "None")
else()
    set(BUILD_TYPE "${CMAKE_BUILD_TYPE}")
endif()

# Collect git info
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    OUTPUT_VARIABLE GIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    set(GIT_HASH "unknown")
    set(GIT_BRANCH "unknown")
endif()

# Buildgen
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Templates/ProjectBuildInfo.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/BuildInfo/BuildInfo.h" @ONLY)

add_library(BuildInfo INTERFACE)
target_include_directories(BuildInfo INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/BuildInfo")
