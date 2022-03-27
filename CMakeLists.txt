cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 11)

set(This mycc)

project(${This} LANGUAGES C)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

file(GLOB Sources src/*.c)
file(GLOB PreprocSources src/preproc/*.c)
file(GLOB ASTSources src/ast/*.c)
file(GLOB ParserSources src/parser/*.c)

set(Lib mycclib)

add_library(${Lib} OBJECT ${Sources} ${PreprocSources} ${ASTSources} ${ParserSources})
target_include_directories(${Lib} PUBLIC ./include)

add_executable(${This} main.c)
target_link_libraries(${This} ${Lib})

set(Test mycctest)

file(GLOB TestSources test/*.c)
file(GLOB PreprocTestSources test/preproc/*.c)
file(GLOB ParserTestSources test/parser/*.c)

add_executable(${Test} ${TestSources} ${PreprocTestSources} ${ParserTestSources})
target_link_libraries(${Test} ${Lib})
