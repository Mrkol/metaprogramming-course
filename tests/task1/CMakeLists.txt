if (NOT NOCOMPILE)
    make_test(main main.cpp)
endif()

if (NOCOMPILE)
    make_test(nocompile nocompile.cpp)
endif()
