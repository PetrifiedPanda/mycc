add_library(mycc-util)
target_include_directories(mycc-util PUBLIC ./include)

add_subdirectory(src)

if (${MYCC_ENABLE_TESTS})
    add_executable(mycc-util-test)
    target_link_libraries(mycc-util-test mycc-util)
    target_link_libraries(mycc-util-test mycc-testing)

    add_test(NAME util COMMAND mycc-util-test WORKING_DIRECTORY ${TestWorkingDir})

    add_subdirectory(test)
endif()
