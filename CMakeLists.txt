
cmake_minimum_required (VERSION 3.4)

project (xorshift
         LANGUAGES CXX C
         VERSION 1.0.0)

list (INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/CMake")

function (setvar_default var_)
    if (NOT DEFINED ${var_})
        set (${var_} ${ARGN} PARENT_SCOPE)
    endif ()
endfunction ()

setvar_default (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
setvar_default (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
setvar_default (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

enable_testing ()

include (cotire OPTIONAL)

set (XORSHIFT_HEADERS include/xorshift.hpp include/xoroshiro.hpp)

add_subdirectory (test)
add_subdirectory (include)

add_library (xorshift INTERFACE)
    target_compile_features (xorshift INTERFACE cxx_range_for)
    target_include_directories (xorshift
                                INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                          $<INSTALL_INTERFACE:include/xorshift>)
