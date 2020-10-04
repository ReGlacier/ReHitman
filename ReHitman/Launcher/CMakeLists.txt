cmake_minimum_required(VERSION 3.16)
project(Launcher CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(Launcher ${CMAKE_CURRENT_SOURCE_DIR}/source/Launcher.cpp)
target_link_libraries(Launcher Kernel32.lib)
add_dependencies(Launcher ReHitman)