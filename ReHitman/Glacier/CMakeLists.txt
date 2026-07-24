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
)

if(MSVC)
    set(CMAKE_ASM_MASM_FLAGS "/c /coff")
endif()

set(GLACIER_ALL_SOURCES ${GLACIER_SOURCES} ${GLACIER_ASM_SOURCES})

# Game target with normal allocators
add_library(Glacier STATIC ${GLACIER_ALL_SOURCES})
add_library(G1::Core ALIAS Glacier)
target_include_directories(Glacier PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Glacier ${GLACIER_COMMON_LIBS})

# Target for testing (STL allocators, no extra deps to game instance)
add_library(GlacierTest STATIC ${GLACIER_ALL_SOURCES})
add_library(G1::TestCore ALIAS GlacierTest)
target_include_directories(GlacierTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GlacierTest ${GLACIER_COMMON_LIBS})
target_compile_definitions(GlacierTest PUBLIC REHITMAN_TESTS)

# Fixes for raw asm
foreach(asm_file ${GLACIER_ASM_SOURCES})
    get_filename_component(asm_name ${asm_file} NAME_WE)
    set_source_files_properties(${asm_file} PROPERTIES 
        COMPILE_FLAGS "/Fo\"$(IntDir)${asm_name}.obj\""
    )
endforeach()

add_subdirectory(Tests)