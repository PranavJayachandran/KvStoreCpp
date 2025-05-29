cmake_minimum_required(VERSION 3.10)
project(KVStore)

set(CMAKE_CXX_STANDARD 17)

# Main executable
file(GLOB SRC_FILES src/*.cpp)

add_executable(KVStore ${SRC_FILES})
target_include_directories(KVStore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# === Unit Testing Setup ===

enable_testing()

# Add GoogleTest subdirectory (assuming you cloned it into third_party/googletest)
add_subdirectory(third_party/googletest)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/googletest/include
)

# Gather test source files
file(GLOB TEST_SRC
    tests/*.cpp
)

# Combine test source files with main source files
add_executable(test_all
  src/file_handler.cpp
  src/config.cpp
  ${TEST_SRC}
)

target_link_libraries(test_all
    gtest
    gtest_main
    pthread
)

add_test(NAME UNITTEST COMMAND test_all)
