set(This mycc-frontend)

file(GLOB Sources src/*.c
                  src/preproc/*.c
                  src/ast/*.c
                  src/ast/declaration/*.c
                  src/parser/*.c)

add_library(${This} OBJECT ${Sources})
target_link_libraries(${This} PUBLIC ${Util})
target_include_directories(${This} PUBLIC ./include)

if (${MYCC_ENABLE_TESTS})
    set(Test mycc-frontend-test)

    file(GLOB TestSources test/*.c
                          test/util/*.c
                          test/preproc/*.c
                          test/parser/*.c)
    add_executable(${Test} ${TestSources})
    target_link_libraries(${Test} ${Frontend})
    target_link_libraries(${Test} ${Util})
    target_link_libraries(${Test} ${Testing})

    add_test(NAME frontend COMMAND ${Test} WORKING_DIRECTORY ${TestWorkingDir})
endif()

