cmake_minimum_required(VERSION 4.0)
project(BloodMoney)

set(CMAKE_CXX_STANDARD 20)

# Glacier and BloodMoney form a single module: both source trees are compiled into the one
# `Glacier` static library. Only the namespaces remain split (Glacier = engine, Hitman = game),
# so either half may call into the other.
file(GLOB_RECURSE HITMAN_BM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
)

target_sources(Glacier PRIVATE ${HITMAN_BM_SOURCES})
target_include_directories(Glacier PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# The test build recompiles the whole module, so it needs the game half as well.
if(TARGET GlacierTest)
    target_sources(GlacierTest PRIVATE ${HITMAN_BM_SOURCES})
    target_include_directories(GlacierTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

# Kept as aliases so existing link statements keep resolving to the merged module.
add_library(BloodMoney ALIAS Glacier)
add_library(Hitman ALIAS Glacier)
