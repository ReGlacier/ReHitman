cmake_minimum_required(VERSION 3.17)
project(Client)
set(CMAKE_CXX_STANDARD 20)

add_library(Client INTERFACE)
add_library(ReHitman::IClient ALIAS Client)

target_include_directories(Client INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)