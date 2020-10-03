project(MIPT_METAPROGRAMMING)
CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)


set(REPOSITORY_PATH "" CACHE STRING "Path to the root of solution repository")
set(TASK "" CACHE STRING "Task id")


if (TASK GREATER 0)
    set(TASK_NAME "task${TASK}")
else()
    message(FATAL_ERROR "Wrong task id format. Use -DTASK option to specify task id.")
endif()

if (REPOSITORY_PATH STREQUAL "")
    message(FATAL_ERROR "Specify path to solution repository with -DREPOSITORY_PATH option.")
else()
    if (IS_ABSOLUTE "${REPOSITORY_PATH}")
        set(SOLUTION_PATH "${REPOSITORY_PATH}/${TASK_NAME}")
    else()
        get_filename_component(SOLUTION_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${REPOSITORY_PATH}/${TASK_NAME}" ABSOLUTE)
    endif()
endif()


message(STATUS "Looking for solution in ${SOLUTION_PATH}")
if(NOT IS_DIRECTORY "${SOLUTION_PATH}")
    message(FATAL_ERROR "No such directory")
endif()

# --------------------

function (make_test name)
    add_executable("${name}" ${ARGN})
    add_test(NAME "${name}" COMMAND "${name}")
endfunction()

# --------------------

message(STATUS "Building tests for ${TASK_NAME}")

enable_testing()
include_directories("include" "${SOLUTION_PATH}")
add_subdirectory("${TASK_NAME}")
