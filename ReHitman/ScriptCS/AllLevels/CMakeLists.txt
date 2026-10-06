cmake_minimum_required(VERSION 3.15)
project(AllLevels LANGUAGES C)

# Components
add_library(AllLevels STATIC
    source/AllLevels.c
    source/Alllevels_Baseboid.c
    source/Alllevels_Bird.c
    source/AllLevels_LevelcontrolParentData.c
    source/Alllevels_Levelcontrol.c
    source/Alllevels_Perceptionconverter.c
    source/Alllevels_Basefunc.c
    source/AlllevelsRatParentData.c
)
target_include_directories(AllLevels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(AllLevels PUBLIC ScriptDllInterface ScriptRuntime)
