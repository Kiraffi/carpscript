cmake_minimum_required(VERSION 3.26)
project(carpscript)

set(CMAKE_CXX_STANDARD 20)

add_executable(carpscript src/main.cpp
        src/mymemory.h
        src/mymemory.cpp
        src/mytypes.h
        src/token.h)
