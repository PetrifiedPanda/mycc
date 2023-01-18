set(This mycc-util)

file(GLOB Sources src/*.c)

add_library(${This} OBJECT ${Sources})
target_include_directories(${This} PUBLIC ./include)

if (${MYCC_TEST} STREQUAL "ON")
    file(GLOB TestSources test/*.c)

    set(Test mycc-util-test)

    add_executable(${Test} ${TestSources})
    target_link_libraries(${Test} ${This})
    target_link_libraries(${Test} ${Testing})

    add_test(NAME util COMMAND ${Test} WORKING_DIRECTORY ${TestWorkingDir})
endif()
