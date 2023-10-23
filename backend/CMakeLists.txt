add_library(mycc-backend)
target_include_directories(mycc-backend PUBLIC ./include)
target_link_libraries(mycc-backend mycc-util)

add_subdirectory(src)
