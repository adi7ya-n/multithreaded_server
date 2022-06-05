cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MultiThreaded_Server VERSION 1.0)

add_executable(server src/server/main.cpp)
target_link_options(server PRIVATE -lboost_log -lboost_thread)
target_include_directories(server PRIVATE src/server/include)