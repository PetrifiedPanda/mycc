#ifndef EXTERNAL_DECLARATION_H
#define EXTERNAL_DECLARATION_H

#include <stdbool.h>

#include "ast/func_def.h"
#include "ast/declaration.h"

typedef struct ExternalDeclaration {
    bool is_func_def;
    union {
        FuncDef func_def;
        Declaration decl;
    };
} ExternalDeclaration;

ExternalDeclaration* create_external_declaration(Declaration decl);
ExternalDeclaration* create_external_declaration_func(FuncDef func_def);

void free_external_declaration_children(ExternalDeclaration* d);

#endif

