cmake_minimum_required(VERSION 3.10)
project(KVStore)

set(CMAKE_CXX_STANDARD 17)

# Main executable
add_executable(KVStore src/main.cpp)
target_include_directories(KVStore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# === Unit Testing Setup ===

# Enable testing features
enable_testing()

# Download or reference GoogleTest
# If you're using a cloned version
add_subdirectory(third_party/googletest)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/googletest/include
)

# Test executable
add_executable(test_all 
  tests/test_memtable.cpp
)

target_link_libraries(test_all
    gtest
    gtest_main
    pthread
)

add_test(NAME UNITTEST COMMAND test_all)

