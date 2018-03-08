cmake_minimum_required(VERSION 3.8)
project(mandel)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES mandel.c mandel-lib.c)
add_executable(mandel ${SOURCE_FILES})
target_link_libraries(mandel m pthread)