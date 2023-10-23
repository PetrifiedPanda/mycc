add_library(mycc-frontend)
target_link_libraries(mycc-frontend PUBLIC mycc-util)
target_include_directories(mycc-frontend PUBLIC ./include)

add_subdirectory(src)

if (${MYCC_ENABLE_TESTS})
    add_subdirectory(test)
endif()

