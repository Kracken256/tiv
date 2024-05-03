cmake_minimum_required(VERSION 3.0)
project(libstiv C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES src/*.c)

add_library(stiv SHARED ${SOURCES})
target_include_directories(stiv PRIVATE src)
target_compile_options(stiv PRIVATE -Wall -Wextra -Werror -pedantic -O3 -ffast-math -fPIC)
target_link_libraries(stiv m)

add_library(stiv_static STATIC ${SOURCES})
target_include_directories(stiv_static PRIVATE src)
target_compile_options(stiv_static PRIVATE -Wall -Wextra -Werror -pedantic -O3 -ffast-math -fPIC)
target_link_libraries(stiv_static m)
