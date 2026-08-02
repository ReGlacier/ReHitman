cmake_minimum_required(VERSION 4.0)
project(BloodMoney)
set(CMAKE_CXX_STANDARD 20)

enable_testing()
file(GLOB_RECURSE GLACIER_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h)

add_executable(GlacierTests ${GLACIER_TEST_SOURCES})
target_include_directories(GlacierTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(GlacierTests PUBLIC G1::TestCore gtest gtest_main)

add_test(NAME GlacierTests COMMAND GlacierTests)
