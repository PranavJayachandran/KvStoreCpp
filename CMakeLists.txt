cmake_minimum_required(VERSION 3.10)

project(KVStore)

add_executable(KVStore src/main.cpp)

target_include_directories(KVStore PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
