cmake_minimum_required(VERSION 3.16)
project(ScriptingRuntime CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(ScriptingRuntime STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ScriptingRuntime.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/MonoBindings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/MonoException.cpp
)

add_library(MonoRuntime STATIC IMPORTED)
set_property(TARGET MonoRuntime PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../Modules/Mono/lib/libmono-static-sgen.lib")

target_include_directories(ScriptingRuntime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../Modules/Mono/include/mono-2.0")
target_include_directories(ScriptingRuntime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ScriptingRuntime PUBLIC
        # MonoRuntime & deps
        MonoRuntime Winmm.lib Ws2_32.lib Version.lib Bcrypt.lib
        # ReHitman & ReGlacier
        Hitman::BloodMoney G1::Core)

# ------- Sandbox ---------
add_executable(MonoRuntime_Sandbox ${CMAKE_CURRENT_SOURCE_DIR}/source/Sandbox.cpp)
target_link_libraries(MonoRuntime_Sandbox PUBLIC ScriptingRuntime)