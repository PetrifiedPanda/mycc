#ifndef AST_BIN_READER
#define AST_BIN_READER

#include "frontend/file_info.h"

#include "translation_unit.h"

struct bin_read_ast_res {
    bool is_valid;
    struct file_info file_info;
    struct translation_unit tl;
};

struct bin_read_ast_res bin_read_ast(FILE* f);

#endif

