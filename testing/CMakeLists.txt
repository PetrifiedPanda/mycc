add_library(mycc-testing INTERFACE)
target_include_directories(mycc-testing INTERFACE ./include)

# All arguments after the fixed ones are additional libraries to link
function(mycc_add_test Name MainFile)
    add_executable(${Name} ${MainFile})
    target_link_libraries(${Name} PRIVATE mycc-testing)
    if (${ARGC} GREATER 2)
        target_link_libraries(${Name} PRIVATE ${ARGN})
    endif()

    add_test(NAME ${Name} COMMAND ${Name} WORKING_DIRECTORY ${TestWorkingDir})
endfunction()
