cmake_minimum_required(VERSION 3.15)
project(Hideout LANGUAGES C)

# Components
add_library(Hideout SHARED
    source/DllMain.c
    source/Hideout.c
    source/Scripts.c
    Hideout.def
)

target_include_directories(Hideout PUBLIC include ../AllLevels/include)
target_link_libraries(Hideout PUBLIC ScriptDllInterface AllLevels ScriptRuntime)
