#ifndef EXTERNAL_DECLARATION_H
#define EXTERNAL_DECLARATION_H

#include <stdbool.h>

#include "frontend/parser/ParserState.h"

#include "FuncDef.h"
#include "Declaration.h"

typedef struct ExternalDeclaration {
    bool is_func_def;
    union {
        FuncDef func_def;
        Declaration decl;
    };
} ExternalDeclaration;

bool parse_external_declaration_inplace(ParserState* s, ExternalDeclaration* res);

void free_external_declaration_children(ExternalDeclaration* d);

#endif

