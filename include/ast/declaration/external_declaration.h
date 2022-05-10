#ifndef EXTERNAL_DECLARATION_H
#define EXTERNAL_DECLARATION_H

#include <stdbool.h>

#include "parser/parser_state.h"

#include "ast/declaration/func_def.h"
#include "ast/declaration/declaration.h"

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
