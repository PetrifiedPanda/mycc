cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 11)

set(This compiler)

project(${This} LANGUAGES C)

file(GLOB Sources src/*.c)
file(GLOB ASTSources src/ast/*.c)

add_executable(${This} main.c ${Sources} ${ASTSources})
target_include_directories(${This} PRIVATE ./include)
target_compile_options(${This} PRIVATE 
    $<$<CXX_COMPILER_ID:MSVC>: /W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -Wall -Wextra -Wpedantic>
)
