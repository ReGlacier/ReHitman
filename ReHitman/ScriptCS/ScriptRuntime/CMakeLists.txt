cmake_minimum_required(VERSION 3.15)
project(ScriptRuntime LANGUAGES C)

# Components
add_library(ScriptRuntime STATIC
    source/ScriptRuntime.c
)

target_include_directories(ScriptRuntime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ScriptRuntime PUBLIC ScriptDllInterface)
