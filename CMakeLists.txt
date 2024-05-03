cmake_minimum_required(VERSION 3.0)
project(tiv C)

add_subdirectory(libstiv)

file(GLOB_RECURSE SOURCES app/*.c)
add_executable(stiv-jpeg ${SOURCES})
target_include_directories(stiv-jpeg PRIVATE libstiv/src)
target_link_libraries(stiv-jpeg stiv_static jpeg)
target_compile_options(stiv-jpeg PRIVATE -Wall -Wextra -Werror -pedantic -std=c99 -O3 -g)
add_dependencies(stiv-jpeg stiv_static)
