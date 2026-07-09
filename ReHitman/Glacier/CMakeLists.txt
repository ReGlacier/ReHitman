cmake_minimum_required(VERSION 4.0)
project(BloodMoney)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE GLACIER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h)

# Game target with normal allocators
add_library(Glacier STATIC ${GLACIER_SOURCES})
add_library(G1::Core ALIAS Glacier)
target_include_directories(Glacier PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Glacier PUBLIC HF::Core)

# Target for testing (STL allocators, no extra deps to game instance)
add_library(GlacierTest STATIC ${GLACIER_SOURCES})
add_library(G1::TestCore ALIAS GlacierTest)
target_include_directories(GlacierTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GlacierTest PUBLIC HF::Core)
target_compile_definitions(GlacierTest PUBLIC REHITMAN_TESTS)

add_subdirectory(Tests)