make_test(static static.cpp)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(static PRIVATE "-Wno-non-template-friend")
endif()
