cmake_minimum_required(VERSION 3.20)
project(dsl-compiler CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dslc
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/sema/sema.cpp
)

