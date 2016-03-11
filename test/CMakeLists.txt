
cmake_minimum_required (VERSION 3.3)

set (TEST_SOURCES main.cpp xorshift128.cpp)

add_executable (test_xorshift ${TEST_SOURCES})
    target_link_libraries (test_xorshift xorshift)

add_test (NAME test_xorshift
          COMMAND test_xorshift -r compact)
