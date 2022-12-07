cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(This mycc)

project(${This} LANGUAGES C)

include(CTest)

if (WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(TestWorkingDir ${CMAKE_CURRENT_BINARY_DIR})

set(Testing mycc-testing)
set(Util mycc-util)
set(Frontend mycc-frontend)
set(Backend mycc-backend)

add_subdirectory(testing)
add_subdirectory(util)
add_subdirectory(frontend)
add_subdirectory(backend)

add_executable(${This} main.c)
target_link_libraries(${This} ${Frontend})
target_link_libraries(${This} ${Util})

