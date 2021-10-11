cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 11)

set(This compiler)

project(${This} LANGUAGES C)

file(GLOB Sources src/*.c)
file(GLOB ASTSources src/ast/*.c)

add_executable(${This} main.c ${Sources} ${ASTSources})
target_include_directories(${This} PRIVATE ./include)
