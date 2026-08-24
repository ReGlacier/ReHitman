cmake_minimum_required(VERSION 4.0)
project(ReHitman)
enable_testing()

set(CMAKE_CXX_STANDARD 20)

set(SI_SOURCES
    # Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Actorcommunication.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Fightcontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Friskbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Guardquartercontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Hm3Gui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Hm3Weapon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_M04Levelcontrol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_M11Levelcontrol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_M12Levelcontrol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_M12LevelcontrolExtra.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Holevelcontrol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Bloodtrail.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Cigs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Zlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Debugfunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Physics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Audio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Istring.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Pathfinder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Osd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Coverlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Pathfollower.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Poscontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Silevelcontrol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_ZActor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_ZCar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_ZHM3Actor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_ZLink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Zgeom.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Zhm3Boid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Zhm3Securitycamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Zusepoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SI_Common.cpp
)

add_library(ScriptDllInterface INTERFACE)
target_include_directories(ScriptDllInterface INTERFACE include)
target_compile_features(ScriptDllInterface INTERFACE cxx_std_20)

add_library(ScriptInterface STATIC ${SI_SOURCES})

if (MSVC)
    target_link_options(ScriptInterface INTERFACE
        "/WHOLEARCHIVE:$<TARGET_FILE:ScriptInterface>"
    )
endif()

target_include_directories(ScriptInterface
    PUBLIC
        include
        ${CMAKE_CURRENT_LIST_DIR}/../BloodMoney/include
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../Glacier/include
)
target_link_libraries(ScriptInterface PUBLIC ScriptDllInterface)


if (REHITMAN_TESTS AND TARGET G1::TestCore)
    # Test env
    target_link_libraries(
        ScriptInterface
        PUBLIC
            G1::TestCore
    )
else()
    # Prod env
    target_link_libraries(
        ScriptInterface
        PUBLIC
            G1::Core
    )
endif()
