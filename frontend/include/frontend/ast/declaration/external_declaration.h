#ifndef EXTERNAL_DECLARATION_H
#define EXTERNAL_DECLARATION_H

#include <stdbool.h>

#include "frontend/parser/parser_state.h"

#include "func_def.h"
#include "declaration.h"

struct external_declaration {
    bool is_func_def;
    union {
        struct func_def func_def;
        struct declaration decl;
    };
};

bool parse_external_declaration_inplace(struct parser_state* s,
                                        struct external_declaration* res);

void free_external_declaration_children(struct external_declaration* d);

#endif

