#ifndef EXTERNAL_DECLARATION_H
#define EXTERNAL_DECLARATION_H

#include <stdbool.h>

#include "ast/func_def.h"
#include "ast/declaration.h"

struct external_declaration {
    bool is_func_def;
    union {
        struct func_def func_def;
        struct declaration decl;
    };
};

struct external_declaration* create_external_declaration(struct declaration decl);
struct external_declaration* create_external_declaration_func(struct func_def func_def);

void free_external_declaration_children(struct external_declaration* d);

#endif

