#ifndef MYCC_FRONTEND_AST_AST_DESERIALIZER_H
#define MYCC_FRONTEND_AST_AST_DESERIALIZER_H

#include "util/File.h"

#include "frontend/FileInfo.h"

#include "TranslationUnit.h"

typedef struct {
    bool is_valid;
    FileInfo file_info;
    TranslationUnit tl;
} DeserializeAstRes;

DeserializeAstRes deserialize_ast(File f);

#endif

