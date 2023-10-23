add_library(mycc-frontend)
target_link_libraries(mycc-frontend PUBLIC mycc-util)
target_include_directories(mycc-frontend PUBLIC ./include)

add_subdirectory(src)

if (${MYCC_ENABLE_TESTS})
    add_executable(mycc-frontend-test)
    target_link_libraries(mycc-frontend-test mycc-frontend)
    target_link_libraries(mycc-frontend-test mycc-util)
    target_link_libraries(mycc-frontend-test mycc-testing)

    add_test(NAME frontend COMMAND mycc-frontend-test WORKING_DIRECTORY ${TestWorkingDir})

    add_subdirectory(test)
endif()

