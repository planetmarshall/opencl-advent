cmake_minimum_required(VERSION 3.21)

project(SampleProject LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wpedantic -Wextra -Wconversion)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang$")
        add_compile_options(-Wconditional-uninitialized)
        if (CMAKE_BUILD_TYPE STREQUAL Debug)
            add_compile_options(-fstandalone-debug)
        endif()
    endif()
endif()

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(OpenCLHeadersCpp REQUIRED)
find_package(range-v3 REQUIRED)

add_subdirectory(src)
