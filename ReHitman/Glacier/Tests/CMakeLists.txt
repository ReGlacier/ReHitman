cmake_minimum_required(VERSION 4.0)
project(BloodMoney)
set(CMAKE_CXX_STANDARD 20)

enable_testing()
file(GLOB_RECURSE GLACIER_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h)

add_executable(GlacierTests ${GLACIER_TEST_SOURCES})
target_include_directories(GlacierTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Absolute location of the bundled test data so tests do not depend on the
# process working directory (CI / IDE runs).
target_compile_definitions(GlacierTests PUBLIC
    GLACIER_TESTS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

target_link_libraries(GlacierTests PUBLIC G1::TestCore gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(GlacierTests
    TEST_PREFIX Glacier.
    PROPERTIES LABELS Glacier
)
