if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT WIN32)
    add_compile_options("-fbracket-depth=1024")
endif ()

make_test(main main.cpp)
make_test(stress stress.cpp)
