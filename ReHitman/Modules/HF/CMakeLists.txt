cmake_minimum_required(VERSION 3.16)
project(HF CXX)
set(CMAKE_CXX_STANDARD 20)

# Root Project
add_library(HF INTERFACE)
add_library(HF::Core ALIAS HF)
target_include_directories(HF INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Tests
add_subdirectory(tests)