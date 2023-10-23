target_sources(mycc-frontend PRIVATE AssignExpr.c
                                     ast.c
                                     AstNodeInfo.c
                                     ast_deserializer.c
                                     ast_dumper.c
                                     ast_dumper_2.c
                                     ast_serializer.c
                                     ast_serializer_2.c
                                     compare_asts.c
                                     Expr.c
                                     GenericSel.c
                                     Identifier.c
                                     Initializer.c
                                     SpecQualList.c
                                     Statement.c
                                     StringLiteralNode.c
                                     TranslationUnit.c
                                     TypeName.c
                                     UnaryExpr.c)

add_subdirectory(declaration)
