#ifndef DIRECT_DECLARATOR_H
#define DIRECT_DECLARATOR_H

#include <stdbool.h>
#include <stddef.h>

#include "ast/param_type_list.h"
#include "ast/identifier_list.h"

typedef struct ConstExpr ConstExpr;
typedef struct Declarator Declarator;

typedef enum {
    ARR_OR_FUNC_ARRAY,
    ARR_OR_FUNC_FUN_TYPES,
    ARR_OR_FUNC_FUN_PARAMS
} ArrOrFuncSuffixType;

typedef struct {
    ArrOrFuncSuffixType type;
    union {
        ConstExpr* arr_len;
        ParamTypeList fun_types;
        IdentifierList fun_params;
    };
} ArrOrFuncSuffix;

typedef struct DirectDeclarator {
    bool is_id;
    union {
        char* id;
        Declarator* decl;
    };
    size_t len;
    ArrOrFuncSuffix* suffixes;
} DirectDeclarator;

DirectDeclarator* create_direct_declarator(char* id, ArrOrFuncSuffix* suffixes, size_t len);
DirectDeclarator* create_direct_declarator_decl(Declarator* decl, ArrOrFuncSuffix* suffixes, size_t len);

void free_direct_declarator(DirectDeclarator* d);

#include "ast/const_expr.h"
#include "ast/declarator.h"

#endif
