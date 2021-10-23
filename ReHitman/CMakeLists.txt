cmake_minimum_required(VERSION 3.16)
project(ReHitman)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-D_CRT_SECURE_NO_WARNINGS=1) #MSVS Stuff

add_subdirectory(Modules/fmt)
add_subdirectory(Modules/spdlog)
add_subdirectory(Modules/ImGui)
add_subdirectory(Modules/HF)
add_subdirectory(Modules/gtest)

add_subdirectory(Glacier)
add_subdirectory(BloodMoney)
add_subdirectory(Launcher)
add_subdirectory(Client)
add_subdirectory(ScriptingRuntime)

file(GLOB_RECURSE REHITMAN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/DLL/source/*.cpp)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(ReHitman SHARED ${REHITMAN_SOURCES})
target_include_directories(ReHitman PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/DLL/include)
target_link_libraries(ReHitman PUBLIC G1::Core ReHitman::IClient spdlog fmt::fmt)

# For BloodMoney only (TODO: Create abstraction for this later)
target_link_libraries(ReHitman PRIVATE Hitman::BloodMoney ImGui::Core ImGui::DX9)
target_compile_definitions(ReHitman PRIVATE -DREHITMAN_BLOOD_MONEY_PROJECT=1) #Build for HitmanBloodMoney