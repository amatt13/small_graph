
cmake_minimum_required(VERSION 3.6)

PROJECT(read_data)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fopenmp ")

include_directories(/home/anders/Documents/small_graph/read_data)

add_executable(main main.cpp)