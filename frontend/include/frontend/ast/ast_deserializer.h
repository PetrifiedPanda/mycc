#ifndef AST_DESERIALIZER_H
#define AST_DESERIALIZER_H

#include "frontend/FileInfo.h"

#include "TranslationUnit.h"

typedef struct {
    bool is_valid;
    FileInfo file_info;
    TranslationUnit tl;
} DeserializeAstRes;

DeserializeAstRes deserialize_ast(FILE* f);

#endif

