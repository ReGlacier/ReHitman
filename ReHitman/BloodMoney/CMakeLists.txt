cmake_minimum_required(VERSION 3.17)
project(BloodMoney)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE HITMAN_BM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.asm)

add_library(BloodMoney STATIC ${HITMAN_BM_SOURCES})
add_library(Hitman::BloodMoney ALIAS BloodMoney)
target_include_directories(BloodMoney PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(BloodMoney ReHitman::IClient spdlog)