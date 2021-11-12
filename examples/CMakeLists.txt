cmake_minimum_required(VERSION 3.20)
project(examples)



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER g++-11)


add_executable(type_lists src/type_lists.cpp)
add_executable(loophole_ubiq src/loophole_ubiq.cpp)
add_executable(constexpr_counter src/constexpr_counter.cpp)
add_executable(polymorphic_fun src/polymorphic_fun.cpp)
add_executable(vtable_fun src/vtable_fun.cpp)
add_executable(test src/test.cpp)
add_executable(tag_invoke src/tag_invoke.cpp)
add_executable(UniquePointer src/UniquePointer.cpp)
