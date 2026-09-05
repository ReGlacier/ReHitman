cmake_minimum_required(VERSION 4.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ReHitmanRoot)
enable_testing()

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
#TODO Check that we will compile only under x86!

add_subdirectory(ReHitman)
