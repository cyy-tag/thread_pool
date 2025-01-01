cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

add_compile_options(-g -O3)
include_directories(include/)

add_subdirectory(benchmark)
add_subdirectory(test)
add_subdirectory(example)
