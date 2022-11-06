#ifndef AST_DESERIALIZER_H
#define AST_DESERIALIZER_H

#include "frontend/file_info.h"

#include "translation_unit.h"

struct deserialize_ast_res {
    bool is_valid;
    struct file_info file_info;
    struct translation_unit tl;
};

struct deserialize_ast_res deserialize_ast(FILE* f);

#endif

