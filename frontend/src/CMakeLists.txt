target_sources(mycc-frontend PRIVATE ArchTypeInfo.c
                                     arg_parse.c
                                     ErrBase.c
                                     ExpectedTokensErr.c
                                     FileInfo.c
                                     StrLit.c
                                     Token.c
                                     Value.c)

add_subdirectory(ast)
add_subdirectory(parser)
add_subdirectory(preproc)
