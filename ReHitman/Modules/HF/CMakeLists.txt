cmake_minimum_required(VERSION 3.17)
project(HF CXX)
set(CMAKE_CXX_STANDARD 20)

# ImGui library

add_library(HF INTERFACE)
add_library(HF::Core ALIAS HF)
target_include_directories(HF INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)