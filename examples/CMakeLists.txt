cmake_minimum_required(VERSION 3.20)
project(examples)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER g++)

add_executable(type_lists src/type_lists.cpp)
add_executable(loophole_ubiq src/loophole_ubiq.cpp)
add_executable(constexpr_counter src/constexpr_counter.cpp)
