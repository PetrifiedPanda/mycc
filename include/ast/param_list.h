#ifndef PARAM_LIST_H
#define PARAM_LIST_H

#include <stddef.h>

typedef struct ParamDeclaration ParamDeclaration;

typedef struct ParamList {
    size_t len;
    ParamDeclaration* decls;
} ParamList;

ParamList* create_param_list(ParamDeclaration* decls, size_t len);

void free_param_list(ParamList* l);

#include "ast/param_declaration.h"

#endif

