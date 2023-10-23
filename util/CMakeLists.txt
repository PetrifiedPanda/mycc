add_library(mycc-util)
target_include_directories(mycc-util PUBLIC ./include)

add_subdirectory(src)

if (MYCC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
