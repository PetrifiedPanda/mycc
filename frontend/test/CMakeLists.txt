add_library(mycc-frontend-test-helper test_helpers.c)
target_link_libraries(mycc-frontend-test-helper PUBLIC mycc-frontend mycc-testing)

add_subdirectory(parser)
add_subdirectory(preproc)
