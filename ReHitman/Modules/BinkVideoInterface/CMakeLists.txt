add_library(BinkVideoInterface STATIC
    BinkVideoInterface.cpp
    BinkVideoInterface.h
)

target_include_directories(BinkVideoInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(BinkVideoInterface PUBLIC cxx_std_20)
