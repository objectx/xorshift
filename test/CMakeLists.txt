
cmake_minimum_required (VERSION 3.3)

set (TEST_SOURCES xorshift128.cpp xoroshiro128.cpp prng.cpp main.cpp)

add_executable (test_xorshift ${TEST_SOURCES})
    target_link_libraries (test_xorshift xorshift)

if (COMMAND cotire)
    cotire (test_xorshift)
endif ()

add_test (NAME test_xorshift
          COMMAND test_xorshift -r compact)
