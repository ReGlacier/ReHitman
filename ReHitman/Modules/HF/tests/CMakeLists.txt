cmake_minimum_required(VERSION 3.16)
project(HFTests)

file(GLOB_RECURSE HFTESTS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(HFTests ${HFTESTS_SOURCES})
target_link_libraries(HFTests PRIVATE gtest gtest_main HF::Core)