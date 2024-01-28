cmake_minimum_required(VERSION 3.10)

project(ThreadPool)

add_definitions(--std=c++17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


add_executable(test example/test.cpp)
target_link_libraries(test pthread)