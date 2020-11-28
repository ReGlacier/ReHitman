cmake_minimum_required(VERSION 3.16)
project(BloodMoney)
set(CMAKE_CXX_STANDARD 20)

file(GLOB GLACIER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_library(Glacier STATIC ${GLACIER_SOURCES})
add_library(G1::Core ALIAS Glacier)
target_include_directories(Glacier PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)