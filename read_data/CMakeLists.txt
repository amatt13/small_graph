cmake_minimum_required(VERSION 2.8)

# name of the project
PROJECT(read_data)

# use C++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp ")

# include the directory that CMakeLists.txt is located
include_directories(${read_data_SOURCE_DIR})

add_executable(main main.cpp)
add_executable(main_statio main.cpp)