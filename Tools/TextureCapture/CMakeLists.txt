cmake_minimum_required(VERSION 3.17)
project(TexCapture)

set(CMAKE_CXX_STANDARD 14)

# Produces nvCPL.dll: the game delay-loads nvCPL.dll and calls the exported
# NvCplGetDataInt during startup, so dropping this dll next to
# HitmanBloodMoney.exe is enough to make it load (same trick as ReHitman/NVCPL).
add_library(nvCPL SHARED ${CMAKE_CURRENT_SOURCE_DIR}/Source/TextureCapture.cpp)
target_include_directories(nvCPL PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
