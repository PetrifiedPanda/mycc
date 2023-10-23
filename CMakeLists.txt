cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(mycc LANGUAGES C)

option(MYCC_ENABLE_TESTS "Enable unit tests" OFF)
option(MYCC_ENABLE_MEMDEBUG "Enable memory debugger" OFF)
option(MYCC_ENABLE_LOG "Enable logger" OFF)

if (WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

if (MYCC_MEMDEBUG)
    add_compile_definitions(MYCC_MEMDEBUG)
endif()

if (MYCC_ENABLE_LOG)
    add_compile_definitions(MYCC_ENABLE_LOG)
endif()

if (MYCC_ENABLE_TESTS)
    include(CTest)
    set(TestWorkingDir ${CMAKE_CURRENT_BINARY_DIR})
    add_compile_definitions(MYCC_TEST_FUNCTIONALITY)
    add_subdirectory(testing)
endif()
add_subdirectory(util)
add_subdirectory(frontend)
add_subdirectory(backend)

add_executable(mycc main.c)
target_link_libraries(mycc mycc-frontend)
