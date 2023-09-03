#ifndef FRONTEND_AST_AST_SERIALIZER_2_H
#define FRONTEND_AST_AST_SERIALIZER_2_H

#include "ast.h"

typedef struct {
    AST ast;
    FileInfo file_info;
} DeserializeASTRes_2;

DeserializeASTRes_2 deserialize_ast_2(File f);

bool serialize_ast_2(const AST* ast, const FileInfo* file_info, File f);

#endif
