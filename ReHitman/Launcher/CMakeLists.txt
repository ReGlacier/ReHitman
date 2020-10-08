cmake_minimum_required(VERSION 3.16)
project(Launcher CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(Launcher ${CMAKE_CURRENT_SOURCE_DIR}/source/Launcher.cpp)
target_link_libraries(Launcher Kernel32.lib)

#Uncomment next line to enable pause before process will be resumed
target_compile_definitions(Launcher PRIVATE -DLAUNCHER_DEBUG_MEMORY_ENABLED=1)
set_target_properties(Launcher PROPERTIES OUTPUT_NAME "Launch.ReHitman")

add_dependencies(Launcher ReHitman)