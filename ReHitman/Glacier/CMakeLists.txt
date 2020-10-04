cmake_minimum_required(VERSION 3.17)
project(BloodMoney)
set(CMAKE_CXX_STANDARD 20)

add_library(Glacier INTERFACE)
add_library(G1::Core ALIAS Glacier)
target_include_directories(Glacier INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)