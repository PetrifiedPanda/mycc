#ifndef FRONTEND_AST_AST_SERIALIZER_2_H
#define FRONTEND_AST_AST_SERIALIZER_2_H

#include "ast.h"

typedef struct {
    AST ast;
    FileInfo file_info;
} DeserializeASTRes;

DeserializeASTRes deserialize_ast(File f);

bool serialize_ast(const AST* ast, const FileInfo* file_info, File f);

#endif
