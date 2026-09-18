cmake_minimum_required(VERSION 4.0)
project(ReHitmanProject)
enable_testing()

set(CMAKE_CXX_STANDARD 20)

add_definitions(-D_CRT_SECURE_NO_WARNINGS=1) #MSVS Stuff

add_subdirectory(Modules/fmt)
add_subdirectory(Modules/spdlog)
add_subdirectory(Modules/ImGui)
add_subdirectory(Modules/HF)
add_subdirectory(Modules/gtest)
add_subdirectory(Modules/zlib)
add_subdirectory(Modules/stlport)
add_subdirectory(Modules/jpeg-6b)
add_subdirectory(Modules/BinkVideoInterface)
add_subdirectory(Modules/Quazal)

add_subdirectory(BuildInfo)
add_subdirectory(Glacier)
add_subdirectory(BloodMoney)
add_subdirectory(ScriptInterface)
add_subdirectory(ReHitman)
add_subdirectory(ScriptCS)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
