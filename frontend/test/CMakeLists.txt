target_sources(mycc-frontend-test PRIVATE frontend_test.c
                                          test_helpers.c)

add_subdirectory(parser)
add_subdirectory(preproc)
