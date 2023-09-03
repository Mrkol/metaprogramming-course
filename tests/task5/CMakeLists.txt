if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # May possibly work?
    add_compile_options("/fsanitize=address")
else ()
    add_compile_options("-fsanitize=address,undefined")
    add_link_options("-fsanitize=address,undefined")
endif ()

make_test(main main.cpp)
make_test(static static.cpp)
make_test(sbo sbo.cpp)
