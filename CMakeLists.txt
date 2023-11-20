cmake_minimum_required(VERSION 3.22)
project(carpscript)

set(CMAKE_CXX_STANDARD 20)

add_executable(carpscript src/main.cpp
        src/common.cpp
        src/common.h
        src/compiler.cpp
        src/debug.cpp
        src/debug.h
        src/error.h
        src/mymemory.cpp
        src/mymemory.h
        src/mytypes.h
        src/nativefns.h
        src/nativefns.cpp
        src/op.h
        src/script.cpp
        src/script.h
        src/token.h
        src/vm.cpp
        src/vm.h
        src/scanner.cpp

        src/main_old.cpp
)

#set_target_properties(carpscript
#   PROPERTIES
#   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
#)

