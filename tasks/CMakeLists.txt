CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

project(METAPROGRAMMING)
set(CMAKE_CXX_STANDARD 20)

# deps
# --------------------
include(../third_party/get_deps.cmake)
# --------------------

set(REPOSITORY_PATH "" CACHE STRING "Path to the root of solution repository")
set(TASK "" CACHE STRING "Task name")
option(NOCOMPILE "Build `nocompile` tests in addition to normal ones; `nocompile` tests must not compile" OFF)


if (TASK STREQUAL "")
  message(FATAL_ERROR "Specify task name with -DTASK option.")
endif()

if (REPOSITORY_PATH STREQUAL "")
  message(FATAL_ERROR "Specify path to solution repository with -DREPOSITORY_PATH option.")
else()
  cmake_path(SET REPOSITORY_PATH NORMALIZE ${REPOSITORY_PATH})
  if (IS_ABSOLUTE "${REPOSITORY_PATH}")
    set(SOLUTION_PATH "${REPOSITORY_PATH}/${TASK}")
  else()
    get_filename_component(SOLUTION_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${REPOSITORY_PATH}/${TASK}" ABSOLUTE)
  endif()
endif()


message(STATUS "Looking for solution in ${SOLUTION_PATH}")
if(NOT IS_DIRECTORY "${SOLUTION_PATH}")
    message(FATAL_ERROR "No such directory")
endif()

# --------------------

function (make_test name)
    add_executable("${name}" ${ARGN})
    target_link_libraries("${name}" gtest gtest_main gmock)
    add_test(NAME "${name}" COMMAND "${name}")
endfunction()

# --------------------

message(STATUS "Building tests for ${TASK}")

enable_testing()
add_compile_options(-Wall -Wextra)
include_directories("../course" "${SOLUTION_PATH}")
add_subdirectory("${TASK}/tests")
