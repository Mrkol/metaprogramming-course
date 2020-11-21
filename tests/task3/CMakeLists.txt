if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    make_test(main main.cpp)
    target_compile_options(main PRIVATE "-Wno-unused-value")

    make_test(static static.cpp)
else()
    message(FATAL_ERROR "Use GCC to build tests for task 3")
endif()