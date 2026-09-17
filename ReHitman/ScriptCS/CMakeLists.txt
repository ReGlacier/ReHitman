cmake_minimum_required(VERSION 3.15)
project(ReHitman LANGUAGES C)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

# Components
add_subdirectory(AllLevels)
add_subdirectory(Hideout)
