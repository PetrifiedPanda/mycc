#ifndef DIRECT_ABSTRACT_DECL_H
#define DIRECT_ABSTRACT_DECL_H

#include <stdbool.h>
#include <stddef.h>

#include "ast/param_type_list.h"

typedef struct AbstractDeclarator AbstractDeclarator;
typedef struct ConstExpr ConstExpr;

typedef enum {
    DIRECT_ABSTRACT_DECL_ARRAY,
    DIRECT_ABSTRACT_DECL_ABSTRACT_DECL,
    DIRECT_ABSTRACT_DECL_PARAM_TYPE_LIST
} DirectAbstractDeclType;

typedef struct {
    bool is_array_decl;
    union {
        ConstExpr* array_size;
        ParamTypeList func_types;
    };
} ArrayOrFuncSuffix;

typedef struct DirectAbstractDeclarator {
    DirectAbstractDeclType type;
    union {
        ConstExpr* array_size;
        AbstractDeclarator* bracket_decl;
        ParamTypeList func_types;
    };
    
    size_t len;
    ArrayOrFuncSuffix* following_suffixes;
} DirectAbstractDeclarator;

DirectAbstractDeclarator* create_direct_abstract_declarator_arr(ConstExpr* array_size, ArrayOrFuncSuffix* following_suffixes, size_t len);
DirectAbstractDeclarator* create_direct_abstract_declarator_abs_decl(AbstractDeclarator* bracket_decl, ArrayOrFuncSuffix* following_suffixes, size_t len);
DirectAbstractDeclarator* create_direct_abstract_declarator_param_list(ParamTypeList func_types, ArrayOrFuncSuffix* following_suffixes, size_t len);

void free_direct_abstract_declarator(DirectAbstractDeclarator* d);

#include "ast/abstract_declarator.h"
#include "ast/const_expr.h"

#endif
