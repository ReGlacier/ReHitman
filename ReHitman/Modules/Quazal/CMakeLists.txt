add_library(Quazal STATIC
    source/Quazal.cpp
    source/RefCountedObject.cpp
)

target_include_directories(Quazal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(Quazal PUBLIC cxx_std_20)
