cmake_minimum_required(VERSION 4.0)
project(stlport461 LANGUAGES CXX)

add_library(stlport INTERFACE)

set(STLPORT461_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/STLport-4.6.1)
set(STLPORT461_STAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/STLport-4.6.1-stage)

file(COPY ${STLPORT461_SOURCE_DIR}/stlport DESTINATION ${STLPORT461_STAGE_DIR})

if(MSVC)
    get_filename_component(MSVC_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    get_filename_component(MSVC_HOST_DIR ${MSVC_BIN_DIR} DIRECTORY)
    get_filename_component(MSVC_BIN_ROOT_DIR ${MSVC_HOST_DIR} DIRECTORY)
    get_filename_component(MSVC_TOOLSET_DIR ${MSVC_BIN_ROOT_DIR} DIRECTORY)
    set(MSVC_NATIVE_INCLUDE_DIR ${MSVC_TOOLSET_DIR}/include)
    set(WINDOWS_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
    if(NOT WINDOWS_SDK_VERSION)
        set(WINDOWS_SDK_VERSION 10.0.26100.0)
    endif()

    if(DEFINED ENV{UniversalCRTSdkDir})
        set(WINDOWS_UCRT_INCLUDE_DIR "$ENV{UniversalCRTSdkDir}/Include/${WINDOWS_SDK_VERSION}/ucrt")
    else()
        set(WINDOWS_UCRT_INCLUDE_DIR "C:/Program Files (x86)/Windows Kits/10/Include/${WINDOWS_SDK_VERSION}/ucrt")
    endif()

    if(EXISTS ${STLPORT461_STAGE_DIR}/include)
        file(REMOVE_RECURSE ${STLPORT461_STAGE_DIR}/include)
    endif()

    file(MAKE_DIRECTORY ${STLPORT461_STAGE_DIR}/include)
    file(COPY ${MSVC_NATIVE_INCLUDE_DIR}/ DESTINATION ${STLPORT461_STAGE_DIR}/include)
    if(EXISTS ${WINDOWS_UCRT_INCLUDE_DIR})
        file(COPY ${WINDOWS_UCRT_INCLUDE_DIR}/ DESTINATION ${STLPORT461_STAGE_DIR}/include)
    endif()
endif()

if(MSVC)
    # Keep the native STL ahead of STLport's compatibility wrappers. This lets
    # normal <vector>/<map> includes keep using std while STLport is selected
    # explicitly with <stlport/vector>, <stlport/map>, etc.
    target_include_directories(stlport INTERFACE
        ${STLPORT461_STAGE_DIR}
        ${MSVC_NATIVE_INCLUDE_DIR}
        ${STLPORT461_STAGE_DIR}/include
        ${STLPORT461_STAGE_DIR}/stlport
    )
else()
    target_include_directories(stlport INTERFACE
        ${STLPORT461_STAGE_DIR}
        ${STLPORT461_STAGE_DIR}/stlport
    )
endif()

# The original engine used STLport 4.6.1. We only need STL containers here;
# building/linking STLport iostreams is unnecessary and makes modern MSVC integration harder.
target_compile_definitions(stlport INTERFACE
    _STLP_NO_IOSTREAMS=1
    _STLP_VENDOR_GLOBAL_CSTD=1
    _STLP_NO_CSTD_FUNCTION_IMPORTS=1
)

option(REHITMAN_STLPORT_LAYOUT_PROBE "Build STLport 4.6.1 layout probe" ON)

if(REHITMAN_STLPORT_LAYOUT_PROBE)
    add_executable(STLportLayoutProbe STLportLayoutProbe.cpp)
    target_link_libraries(STLportLayoutProbe PRIVATE stlport)
    target_include_directories(STLportLayoutProbe BEFORE PRIVATE
        ${STLPORT461_STAGE_DIR}/stlport
        ${STLPORT461_STAGE_DIR}/include
    )
    set_target_properties(STLportLayoutProbe PROPERTIES
        CXX_STANDARD 98
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    if(MSVC)
        target_compile_options(STLportLayoutProbe PRIVATE /permissive /W0)
    endif()

    add_test(NAME STLportLayoutProbe COMMAND STLportLayoutProbe)

    add_executable(STLportInteropProbe STLportInteropProbe.cpp)
    target_link_libraries(STLportInteropProbe PRIVATE stlport)
    target_include_directories(STLportInteropProbe PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Glacier/include
    )
    set_target_properties(STLportInteropProbe PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    add_test(NAME STLportInteropProbe COMMAND STLportInteropProbe)
endif()
