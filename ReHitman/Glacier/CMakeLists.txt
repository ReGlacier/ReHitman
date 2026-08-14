cmake_minimum_required(VERSION 4.0)
project(BloodMoney LANGUAGES C CXX ASM_MASM)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE GLACIER_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
)

set(GLACIER_ASM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EventBase/ScriptCoroutineScheduler.asm
)

# Common dependencies (put all dependencies here!)
set(GLACIER_COMMON_LIBS 
    PRIVATE 
        zlib
    PUBLIC
        HF::Core
        stlport
)

set(GLACIER_COMMON_INCLUDE_DIRS)

# Windows specific dependencies
if(WIN32)
    if(DEFINED ENV{DXSDK_DIR})
        file(TO_CMAKE_PATH "$ENV{DXSDK_DIR}" DXSDK_DIR_PATH)
        
        set(DXSDK_INCLUDE_DIR "${DXSDK_DIR_PATH}/Include")
        
        if(CMAKE_CL_64)
            set(DXSDK_LIB_DIR "${DXSDK_DIR_PATH}/Lib/x64")
        else()
            set(DXSDK_LIB_DIR "${DXSDK_DIR_PATH}/Lib/x86")
        endif()
    endif()

    list(
        APPEND GLACIER_COMMON_LIBS 
        PUBLIC 
            # DirectX 9
            d3d9
            $<$<CONFIG:Debug>:d3dx9d>$<$<NOT:$<CONFIG:Debug>>:d3dx9>
            # DirectInput8
            dinput8 
            dxguid
            # DirectSound
    )

    message(STATUS "[Glacier] DXSDK_DIR from ENV: $ENV{DXSDK_DIR}")
endif()

if(MSVC)
    set(CMAKE_ASM_MASM_FLAGS "/c /coff")
endif()

set(GLACIER_ALL_SOURCES ${GLACIER_SOURCES} ${GLACIER_ASM_SOURCES})

option(REHITMAN_BUILD_TESTS "Build ReHitman tests" ON)

# Game target
add_library(Glacier STATIC ${GLACIER_ALL_SOURCES})
add_library(G1::Core ALIAS Glacier)

target_include_directories(Glacier PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    $<$<BOOL:${DXSDK_INCLUDE_DIR}>:${DXSDK_INCLUDE_DIR}>
)

if(DXSDK_LIB_DIR)
    target_link_directories(Glacier PUBLIC ${DXSDK_LIB_DIR})
endif()

target_link_libraries(Glacier ${GLACIER_COMMON_LIBS})
target_compile_definitions(Glacier PUBLIC DIRECTINPUT_VERSION=0x0800)

# Test target
if(REHITMAN_BUILD_TESTS)
    add_library(GlacierTest STATIC ${GLACIER_ALL_SOURCES})
    add_library(G1::TestCore ALIAS GlacierTest)

    target_include_directories(GlacierTest PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include 
        $<$<BOOL:${DXSDK_INCLUDE_DIR}>:${DXSDK_INCLUDE_DIR}>
    )

    if(DXSDK_LIB_DIR)
        target_link_directories(GlacierTest PUBLIC ${DXSDK_LIB_DIR})
    endif()

    target_link_libraries(GlacierTest ${GLACIER_COMMON_LIBS})
    target_compile_definitions(GlacierTest PUBLIC REHITMAN_TESTS DIRECTINPUT_VERSION=0x0800)
endif()

# Fixes for raw asm
if(MSVC)
    set(GLACIER_ASM_TARGETS Glacier)
    if(TARGET GlacierTest)
        list(APPEND GLACIER_ASM_TARGETS GlacierTest)
    endif()

    foreach(target_name ${GLACIER_ASM_TARGETS})
        foreach(config_name ${CMAKE_CONFIGURATION_TYPES})
            file(MAKE_DIRECTORY
                ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.dir/${config_name}/source/EventBase
            )
        endforeach()
    endforeach()
endif()

foreach(asm_file ${GLACIER_ASM_SOURCES})
    get_filename_component(asm_name ${asm_file} NAME_WE)
    set_source_files_properties(${asm_file} PROPERTIES 
        COMPILE_FLAGS "/Fo\"$(IntDir)${asm_name}.obj\""
    )
endforeach()

if(REHITMAN_BUILD_TESTS)
    add_subdirectory(Tests)
endif()
