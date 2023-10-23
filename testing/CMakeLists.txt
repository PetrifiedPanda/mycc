add_library(mycc-testing INTERFACE)
target_include_directories(mycc-testing INTERFACE ./include)

function(mycc_add_test name main_file)
    add_executable(${name} ${main_file})
    target_link_libraries(${name} mycc-testing)
    if (${ARGC} GREATER 2)
        target_link_libraries(${name} ${ARGN})
    endif()

    add_test(NAME ${name} COMMAND ${name} WORKING_DIRECTORY ${TestWorkingDir})
endfunction()
