cmake_minimum_required(VERSION 3.17)
project(NVCPL)

set(CMAKE_CXX_STANDARD 14)

add_library(NVCPL SHARED ${CMAKE_CURRENT_SOURCE_DIR}/Source/NVCPL.cpp)