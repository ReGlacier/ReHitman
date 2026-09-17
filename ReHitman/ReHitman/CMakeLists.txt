cmake_minimum_required(VERSION 4.0)
project(ReHitman)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE REHITMAN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c)

# Build meta info
if(NOT DEFINED PROJECT_VERSION_MAJOR OR PROJECT_VERSION_MAJOR STREQUAL "")
    set(PROJECT_VERSION_MAJOR 1)
endif()
if(NOT DEFINED PROJECT_VERSION_MINOR OR PROJECT_VERSION_MINOR STREQUAL "")
    set(PROJECT_VERSION_MINOR 0)
endif()
if(NOT DEFINED PROJECT_VERSION_PATCH OR PROJECT_VERSION_PATCH STREQUAL "")
    set(PROJECT_VERSION_PATCH 0)
endif()

set(PROJECT_VERSION_BUILD 0)
set(COMPANY_NAME "ReHitman")
set(FILE_DESCRIPTION "ReHitman executable")
set(PRODUCT_NAME "ReHitman Project")
string(TIMESTAMP CURRENT_YEAR "%Y")

configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/resources/Version.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/version.rc" @ONLY)

# Add some win32 stuff
if(WIN32)
    list(APPEND REHITMAN_SOURCES
        "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
        "resources/App.rc"
    )
endif()

add_executable(ReHitman WIN32 ${REHITMAN_SOURCES})
target_include_directories(ReHitman PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ReHitman PRIVATE BuildInfo G1::Core BloodMoney)
