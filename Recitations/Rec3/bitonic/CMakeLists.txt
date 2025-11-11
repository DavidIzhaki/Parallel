cmake_minimum_required(VERSION 3.22)
project(bitonic C)

set(CMAKE_C_STANDARD 99)

add_executable(bitonic recursive_bitonic_sort.c)
