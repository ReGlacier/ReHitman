cmake_minimum_required(VERSION 3.15)
project(AllLevels LANGUAGES C)

# Components
add_library(AllLevels STATIC source/AllLevels.c)
target_include_directories(AllLevels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(AllLevels PUBLIC ScriptDllInterface ScriptRuntime)
