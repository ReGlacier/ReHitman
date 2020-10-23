cmake_minimum_required(VERSION 3.16)
project(ImGui)
set(CMAKE_CXX_STANDARD 20)

# ImGui library

add_library(ImGui STATIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
                         ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
                         ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h
                         ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h)
add_library(ImGui::Core ALIAS ImGui)
target_include_directories(ImGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

# Extensions (DirectX 9 support)
add_library(ImGuiDx9 STATIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/examples/imgui_impl_dx9.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/imgui/examples/imgui_impl_win32.cpp)
add_library(ImGui::DX9 ALIAS ImGuiDx9)
target_include_directories(ImGuiDx9 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/examples)
target_link_libraries(ImGuiDx9 PUBLIC ImGui kernel32.lib d3d9.lib)