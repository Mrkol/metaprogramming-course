cmake_minimum_required(VERSION 3.20)

project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(main main.cpp)
add_compile_options("-stdlib=libc++ -fsanitize=address,undefined")
