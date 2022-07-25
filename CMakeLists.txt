cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 17)

set(This mycc)

project(${This} LANGUAGES C)

if (WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

file(GLOB Sources src/*.c
                  src/util/*.c
                  src/preproc/*.c
                  src/ast/*.c
                  src/ast/expr/*.c
                  src/ast/statement/*.c
                  src/ast/declaration/*.c
                  src/ast/initializer/*.c
                  src/parser/*.c)

set(Lib mycclib)

add_library(${Lib} OBJECT ${Sources})
target_include_directories(${Lib} PUBLIC ./include)

add_executable(${This} main.c)
target_link_libraries(${This} ${Lib})

set(Test mycctest)

file(GLOB TestSources test/*.c
                      test/util/*.c
                      test/preproc/*.c
                      test/parser/*.c)

add_executable(${Test} ${TestSources})
target_link_libraries(${Test} ${Lib})
